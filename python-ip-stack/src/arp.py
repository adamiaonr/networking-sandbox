import struct
import binascii
import ipaddress

from ethernet import Ethernet, mac_to_str
from metaframe import MetaFrame
from collections import defaultdict

class ARP_IPv4_Entry:

    def __init__(self, hwtype, sip, smac, state = True):
        
        self.hwtype = hwtype
        self.sip = sip
        self.smac = smac
        self.state = state

    def __str__(self):
        return ("type: %s, src ip: %s, src mac: %s, state: %s" % 
            (self.hwtype, str(ipaddress.IPv4Address(self.sip)), mac_to_str(self.smac), self.state))

    def update(self, arp_data):

        self.sip = arp_data['sip']['value']
        self.smac = arp_data['smac']['value']

class ARP_IPv4_Data():

    def __init__(self, src_mac = '', sip = 0, dst_mac = '', dip = 0):

        self.smac   = src_mac
        self.sip    = sip
        self.dmac   = dst_mac
        self.dip    = dip

    def pack(self):

        values = [self.smac, self.sip, self.dmac, self.dip]
        return struct.pack('! 6s I 6s I', *values)

    def unpack(self, raw_data):

        unpacked_data = struct.unpack('! 6s I 6s I', raw_data)

        self.smac = unpacked_data[0]
        self.sip = unpacked_data[1]
        self.dmac = unpacked_data[2]
        self.dip = unpacked_data[3]

class ARP_Dgram(MetaFrame):

    # size of ARP header and footer (fixed)
    ARP_HDR_SIZE = 0x08
    ARP_FTR_SIZE = 0x00
    # ARP opcodes
    ARP_REQUEST = 0x0001
    ARP_REPLY = 0x0002  
    # ARP protocol types
    ARP_PROTYPE_IPv4 = 0x0800
    ARP_PROTYPE_ARP  = 0x0806    

    def __init__(self, 
        hwtype = 0x0001, protype = 0x0800,
        hwsize = 0x06, prosize = 0x04,
        opcode = 0x0000,
        arp_data = None, arp_data_size = 0):

        MetaFrame.__init__(self)

        self.hdr_size = ARP_Dgram.ARP_HDR_SIZE
        self.ftr_size = ARP_Dgram.ARP_FTR_SIZE

        self.frame['header']['hwtype']  = {'size': 1, 'type': 'H', 'value': hwtype}
        self.frame['header']['protype'] = {'size': 1, 'type': 'H', 'value': protype}
        self.frame['header']['hwsize']  = {'size': 1, 'type': 'B', 'value': hwsize};
        self.frame['header']['prosize'] = {'size': 1, 'type': 'B', 'value': prosize};
        self.frame['header']['opcode']  = {'size': 1, 'type': 'H', 'value': opcode};

        # arp_data is a ARP_IPv4_Data object, so we convert it now to 
        # a byte array
        if arp_data:
            data = arp_data.pack()
            self.frame['data']['data'] = {'size': len(data), 'type': 's', 'value': data};
        else:
            self.frame['data']['data'] = {'size': 0, 'type': 's', 'value': ''};

class ARP_Module:

    def __init__(self, stack):

        self.stack = stack

        # since clients will query using L3 addresses, keys are the sip
        self.arp_table = defaultdict(ARP_IPv4_Entry)
        # set of L2 and L3 types supported by the daemon. by default, let's 
        # add Ethernet for L2 (0x0001) and IPv4 for L3 (0x0800) 
        self.hwtypes    = set([0x0001])
        self.protypes   = set([0x0800])

    def print_table(self):

        for entry in self.arp_table:
            print("[%s, %s] : %s" % (entry[0], str(ipaddress.IPv4Address(entry[1])), str(self.arp_table[entry])))

    def update_arp_table(self, arp_dgram, arp_data):

        # set merge_flag to False
        merge_flag = False
        # the keys of the arp_table dict are a 2-tuple of the form 
        # <protype, source protocol address>
        arp_table_key = (arp_dgram.get_attr('header', 'protype'), arp_data.sip)

        if arp_table_key in self.arp_table:

            self.arp_table[arp_table_key].smac = arp_data.smac
            merge_flag = True

        else:
            print("ARP_Module::update_arp_table() : [WARNING] %s not in ARP table" % (str(ipaddress.IPv4Address(arp_table_key[1]))))

        self.print_table()

        return merge_flag

    def get_record(self, protype, sip):

        if (protype, sip) not in self.arp_table:
            return None
        else:
            return self.arp_table[(protype, sip)]
        
    def send_req(self, dip):
        
        arp_req = ARP_Dgram()
        
        arp_req_data = ARP_IPv4_Data()
        arp_req_data.sip = self.stack.ip
        arp_req_data.dip = dip
        arp_req_data.smac = binascii.unhexlify(self.stack.mac.replace(':', ''))
        # ARP requests set dmac to 00:00:00:00:00:00
        arp_req_data.dmac = binascii.unhexlify('000000000000')
        
        raw_data = arp_req_data.pack()
        arp_req.set_attr('data', 'data', raw_data, size = len(raw_data))
        arp_req.set_attr('header', 'opcode', ARP_Dgram.ARP_REQUEST)

        # send the arp request using the tap device
        self.stack.send_frame(Ethernet.PROTO_ARP, arp_req_data.dmac, arp_req.pack())

    def process_dgram(self, raw_dgram):

        # read raw ARP dgram into ARP_Dgram object
        arp_dgram = ARP_Dgram()
        arp_dgram.unpack(raw_dgram)

        # we handle an ARP dgram according to the algorithm shown in 
        # http://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/

        # is hwtype supported by this arp daemon? if not, abort
        if arp_dgram.get_attr('header', 'hwtype') not in self.hwtypes:
            return

        # is protype supported by this arp daemon? if not, abort
        if arp_dgram.get_attr('header', 'protype') not in self.protypes:
            return

        # check if an entry in arp_table needs to be updated, using the 
        # update_arp_table() method
        arp_data = ARP_IPv4_Data()
        arp_data.unpack(arp_dgram.get_attr('data', 'data'))
        # note that regardless of the destination ip of this ARP dgram,
        # the table is updated (however, it doesn't tell anything about
        # destination mac address of the Ethernet frame which contains it)
        merge_flag = self.update_arp_table(arp_dgram, arp_data)

        # is this arp frame destined to this node? if not, abort
        if arp_data.dip == self.stack.ip:

            # if merge_flag is false, add a new entry to arp_table
            if not merge_flag:

                # generate the new key
                arp_table_key = (arp_dgram.get_attr('header', 'protype'), arp_data.sip)
                # add the table entry
                self.arp_table[arp_table_key] = ARP_IPv4_Entry(
                    hwtype = arp_dgram.get_attr('header', 'hwtype'),
                    sip = arp_data.sip,
                    smac = arp_data.smac)
                
                self.print_table()

            # if arp dgram is an arp request
            if arp_dgram.get_attr('header', 'opcode') == ARP_Dgram.ARP_REQUEST:

                # swap the dgram's hw and protocol fields, putting the local 
                # hw and protocol fields as smac and sip
                arp_data.dip = arp_data.sip
                arp_data.dmac = arp_data.smac
                arp_data.sip = self.stack.ip
                arp_data.smac = binascii.unhexlify(self.stack.mac.replace(':', ''))

                raw_data = arp_data.pack()
                arp_dgram.set_attr('data', 'data', raw_data, size = len(raw_data))
                arp_dgram.set_attr('header', 'opcode', ARP_Dgram.ARP_REPLY)

                # send the arp reply using the tap device
                self.stack.send_frame(Ethernet.PROTO_ARP, arp_data.dmac, arp_dgram.pack())
