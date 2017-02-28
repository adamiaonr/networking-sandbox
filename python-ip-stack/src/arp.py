import struct
import binascii
import collections
import ipaddress

from ethernet import Ethernet
from metaframe import MetaFrame
from collections import defaultdict

class ARP_IPv4_Entry:

    def __init__(self, hwtype, sip, smac, state = True):
        
        self.hwtype = hwtype
        self.sip = sip
        self.smac = smac
        self.state = state

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

class ARP_Packet(MetaFrame):

    ARP_HDR_SIZE = 0x08
    ARP_FTR_SIZE = 0x00

    ARP_REQUEST = 0x0001
    ARP_REPLY = 0x0002  

    def __init__(self, 
        hwtype = 0x0001, protype = 0x0800,
        hwsize = 0x06, prosize = 0x04,
        opcode = 0x0000,
        arp_data = None, arp_data_size = 0):

        MetaFrame.__init__(self)

        self.hdr_size = ARP_Packet.ARP_HDR_SIZE
        self.ftr_size = ARP_Packet.ARP_FTR_SIZE

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

class ARP_Daemon:

    def __init__(self, tap):

        self.tap = tap
        self.mac = tap.dev.hwaddr
        print(tap.dev.addr)
        self.ip = int(ipaddress.IPv4Address(unicode(tap.dev.addr)))

        # since clients will query using L3 addresses, keys are the sip
        self.arp_table = defaultdict(ARP_IPv4_Entry)

        # set of L2 and L3 types supported by the daemon. by default, let's 
        # add Ethernet for L2 (0x0001) and IPv4 for L3 (0x0800) 
        self.hwtypes    = set([0x0001])
        self.protypes   = set([0x0800])

    def update_arp_table(self, arp_packet, arp_data):

        # set merge_flag to False
        merge_flag = False
        # the keys of the arp_table dict are a 2-tuple of the form 
        # <protype, source protocol address>
        arp_table_key = (arp_packet.get_attr('header', 'protype'), arp_data.sip)

        if arp_table_key in self.arp_table:

            self.arp_table[arp_table_key].smac = arp_data.smac
            merge_flag = True

        else:
            print("ARP_Daemon::update_arp_table() : [WARNING] %s not in ARP table" % (str(arp_table_key)))

        return merge_flag

    def process_arp_packet(self, raw_packet):

        # read raw ARP packet into ARP_Packet object
        arp_packet = ARP_Packet()
        arp_packet.unpack(raw_packet)

        # we handle an ARP packet according to the algorithm shown in 
        # http://www.saminiir.com/lets-code-tcp-ip-stack-1-ethernet-arp/

        # is hwtype supported by this arp daemon? if not, abort
        if arp_packet.get_attr('header', 'hwtype') not in self.hwtypes:
            return

        # is protype supported by this arp daemon? if not, abort
        if arp_packet.get_attr('header', 'protype') not in self.protypes:
            return

        # check if an entry in arp_table needs to be updated, using the 
        # update_arp_table() method
        arp_data = ARP_IPv4_Data()
        arp_data.unpack(arp_packet.get_attr('data', 'data'))
        merge_flag = self.update_arp_table(arp_packet, arp_data)

        # is this arp frame destined to this node? if not, abort
        if arp_data.dip == self.ip:

            # if merge_flag is false, add a new entry to arp_table
            if not merge_flag:

                # generate the new key
                arp_table_key = (arp_packet.get_attr('header', 'protype'), arp_data.sip)
                # add the table entry
                self.arp_table[arp_table_key] = ARP_IPv4_Entry(
                    hwtype = arp_packet.get_attr('header', 'hwtype'),
                    sip = arp_data.sip,
                    smac = arp_data.smac)

            # if arp packet is an arp request
            if arp_packet.get_attr('header', 'opcode') == ARP_Packet.ARP_REQUEST:

                # swap the packet's hw and protocol fields, putting the local 
                # hw and protocol fields as smac and sip
                arp_data.dip = arp_data.sip
                arp_data.dmac = arp_data.smac
                arp_data.sip = self.ip
                arp_data.smac = self.mac

                raw_data = arp_data.pack()
                arp_packet.set_attr('data', 'data', raw_data, size = len(raw_data))
                arp_packet.set_attr('header', 'opcode', ARP_Packet.ARP_REPLY)

                # send the arp reply using the tap device
                self.tap.send_frame(Ethernet.PROTO_ARP, arp_data.dmac, arp_packet.pack())
