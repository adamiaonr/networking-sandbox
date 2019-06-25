from metaframe import MetaFrame
from ipv4 import IPv4_Dgram

import ipaddress

from collections import defaultdict
from collections import OrderedDict

class TCP_Flags():
    
    def __init__(self, flags_hex = 0, flags_dict = {}):
        
        # flag bits
        self.flags_hex = flags_hex
        # flag dict
        self.flags_dict = {
            0 : {'name' : 'fin', 'value' : 0},
            1 : {'name' : 'syn', 'value' : 0},
            2 : {'name' : 'rst', 'value' : 0},
            3 : {'name' : 'psh', 'value' : 0},
            4 : {'name' : 'ack', 'value' : 0},
            5 : {'name' : 'urg', 'value' : 0},
            6 : {'name' : 'ece', 'value' : 0},
            7 : {'name' : 'cwr', 'value' : 0},
            8 : {'name' : 'ns', 'value' : 0}}

        if flags_hex > 0:
            self.flags_dict = TCP_Flags.to_dict(flags_hex)

        elif flags_dict:
            self.flags_dict = flags_dict
            self.flags_hex = TCP_Flags.to_hex(flags_dict)

    @staticmethod
    def to_dict(flags_hex):

        # note : we assume order of flags in flags_hex to be
        # (msb) |ns|cwr|ece|urg|ack|psh|rst|syn|fin| (lsb)
        flags_dict = {
            0 : {'name' : 'fin', 'value' : 0},
            1 : {'name' : 'syn', 'value' : 0},
            2 : {'name' : 'rst', 'value' : 0},
            3 : {'name' : 'psh', 'value' : 0},
            4 : {'name' : 'ack', 'value' : 0},
            5 : {'name' : 'urg', 'value' : 0},
            6 : {'name' : 'ece', 'value' : 0},
            7 : {'name' : 'cwr', 'value' : 0},
            8 : {'name' : 'ns', 'value' : 0}}
        
        if flags_hex == 0:
            return flags_dict

        for f in flags_dict:
            flags_dict[f]['value'] = flags_hex & 0x01
            # right shift flags_hex by 1 bit to read f-th flag
            flags_hex = flags_hex >> 1
            
        return flags_dict

    @staticmethod
    def to_hex(flags_dict = {}):

        if not flags_dict:
            return 0

        flags_hex = 0
        for f in flags_dict:
            # OR flags_hex w/ a f-left-shifted value of flags_dict[f]['value']
            flags_hex = flags_hex | ((flags_dict[f]['value'] & 0x01) << f)

        return flags_hex

    @staticmethod
    def to_str(flags_dict):

        output = ''
        for f in flags_dict:
            output += ('%s : %d, ' % (flags_dict[f]['name'], flags_dict[f]['value']))

        return output

    def __str__(self):

        if not self.flags_dict:
            self.flags_dict = TCP_Flags.to_dict(self.flags_hex)

        return TCP_Flags.to_str(self.flags_dict)

class TCP_Seg(MetaFrame):

    # tcp header is 20 byte long:
    TCP_HDR_SIZE = 20
    TCP_FTR_SIZE = 0

    def __init__(self,
        src_addr = 0,
        dst_addr = 0,
        proto = IPv4_Dgram.IPv4_PROTO_TCP,
        src_port = 0,
        dst_port = 0,
        seq_number = 0,
        ack_number = 0,
        flags = 0,
        window_size = 0,
        urgent_ptr = 0,
        tcp_data = None):

        MetaFrame.__init__(self)

        self.hdr_size = TCP_Seg.TCP_HDR_SIZE
        self.ftr_size = TCP_Seg.TCP_FTR_SIZE

        # add a special pseudo header
        # e.g.: https://en.wikipedia.org/wiki/Transmission_Control_Protocol#Checksum_computation
        self.pseudo_header = OrderedDict()
        self.pseudo_header['src_addr'] = {'size': 1, 'type': 'I', 'value': src_addr}
        self.pseudo_header['dst_addr'] = {'size': 1, 'type': 'I', 'value': dst_addr}
        self.pseudo_header['proto'] = {'size': 1, 'type': 'H', 'value': (proto & 0x00FF)}
        self.pseudo_header['tcp_len'] = {'size': 1, 'type': 'H', 'value': (TCP_Seg.TCP_HDR_SIZE)}

        # src & dst port numbers
        self.frame['header']['src_port']     = {'size': 1, 'type': 'H', 'value': src_port}
        self.frame['header']['dst_port']     = {'size': 1, 'type': 'H', 'value': dst_port}
        # seq & ack numbers
        self.frame['header']['seq_number']   = {'size': 1, 'type': 'I', 'value': seq_number}
        self.frame['header']['ack_number']   = {'size': 1, 'type': 'I', 'value': ack_number}
        
        # a mix of tcp header elements (16 bit):
        #   - hdr length (4) : for now, let's keep it at 5 32 bit words (default)
        #   - reserved bits (3)
        #   - flags (9)
        hdrlen_flags = (0x5000) | flags
        self.frame['header']['hdr_length_flags'] = {'size': 1, 'type': 'H', 'value': hdrlen_flags}
        self.frame['header']['window_size']  = {'size': 1, 'type': 'H', 'value': window_size}
        
        # checksum & urgent pointer
        self.frame['header']['cksum']        = {'size': 1, 'type': 'H', 'value': 0x0000}
        self.frame['header']['urgent_ptr']   = {'size': 1, 'type': 'H', 'value': 0x0000}

        if tcp_data:
            data = tcp_data.pack()
            self.frame['data']['data'] = {'size': len(data), 'type': 's', 'value': data}
            self.pseudo_header['tcp_len'] = {'size': 1, 'type': 'H', 'value': (TCP_Seg.TCP_HDR_SIZE + len(data))}            
        else:
            self.frame['data']['data'] = {'size': 0, 'type': 's', 'value': ''}

        # the checksum is calculated as for the ipv4 datagram, but considering 
        # the whole datagram + pseudo-header
        self.frame['header']['cksum']['value'] = self.get_cksum()

    def __str__(self):

        output = ''
        output += 'tcp pseudo header:\n'
        for k in self.pseudo_header:
            if 'addr' in k:
                output += '\t%s : %s\n' % (k, str(ipaddress.IPv4Address(self.pseudo_header[k]['value'])))
            else:
                output += '\t%s : %d\n' % (k, self.pseudo_header[k]['value'])

        output += 'tcp header:\n'
        for k in self.frame['header']:
            output += '\t%s : %d\n' % (k, self.frame['header'][k]['value'])

        return output

    def get_cksum(self, include_data = True):

        # FIXME : this is used by many protocols, so this should be a general method
        """16 bit one's complement of the one's complement 
        sum of all 16 bit words in the pseudo header, header and data. 
        for purposes of computing the checksum, the value of the checksum field is zero."""

        cksum = 0
        # initialize tcp segment by 'packing' the pseudo header as an array of byte
        tcp_seg = MetaFrame.pack_custom(self.pseudo_header, fields = ['src_addr', 'dst_addr', 'proto', 'tcp_len'])
        # append the header & data
        tcp_seg += self.pack(parts = ['header', 'data'])

        count = len(tcp_seg)
        # transform each byte in tcp_seg to a number in [0, 255]
        dgram_bytes = [ord(byte_str) for byte_str in tcp_seg]
        # keep summing blocks of 2 byte from tcp_seg to get a 16 bit sum
        i = 0
        while (count > 1):

            cksum += (dgram_bytes[i] << 8) + dgram_bytes[i + 1]

            # print("{0:#018b}".format((dgram_bytes[i] << 8) & 0xFFFF))
            # print("{0:#018b}".format(dgram_bytes[i + 1] & 0xFFFF))
            # print("------------------")
            # print("{0:#018b}".format(cksum & 0xFFFF))
            # print("\n")

            count -= 2
            i += 2


        # add left-over byte (if any)
        if count > 0:
            cksum += dgram_bytes[0]
        # print("{0:#018b}".format(cksum & 0xFFFF))

        # now fold result into a 16 bit 1's complement sum
        while (cksum >> 16):
            cksum = (cksum & 0xFFFF) + (cksum >> 16)

        # print("{0:#018b}".format(cksum & 0xFFFF))
        # print("0x%0.2X" % cksum)

        return ((~cksum) & 0xFFFF)
