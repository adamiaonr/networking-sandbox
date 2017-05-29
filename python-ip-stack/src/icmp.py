import struct
import binascii
import collections

from ethernet import Ethernet
from metaframe import MetaFrame
from collections import defaultdict

class ICMP_Packet(MetaFrame):

    # icmp header is 4 byte long:
    #   -# type, e.g. echo, echo reply, etc. (1)
    #   -# code, further describes the type (1)
    #   -# checksum of the ipv4 datagram which encapsulates the icmp header (2)
    ICMP_HDR_SIZE = 0x04
    ICMP_FTR_SIZE = 0x00

    # our stack will support 3 types only : ECHO_REPLY, ECHO_REQUEST and 
    # DESTINATION_UNREACHABLE
    ICMP_TYPE_ECHO_REPLY = 0x00  
    ICMP_TYPE_ECHO_REQUEST = 0x08
    ICMP_TYPE_DESTINATION_UNREACHABLE = 0x03  

    def __init__(self, 
        icmp_type = 0x00, 
        icmp_code = 0x00,
        icmp_data = None, icmp_data_size = 0):

        MetaFrame.__init__(self)

        self.hdr_size = ICMP_Packet.ICMP_HDR_SIZE
        self.ftr_size = ICMP_Packet.ICMP_FTR_SIZE

        self.frame['header']['type']    = {'size': 1, 'type': 'B', 'value': icmp_type}
        self.frame['header']['code']    = {'size': 1, 'type': 'B', 'value': icmp_code}
        self.frame['header']['cksum']   = {'size': 1, 'type': 'H', 'value': 0x0000};

        if icmp_data:
            data = icmp_data.pack()
            self.frame['data']['data'] = {'size': len(data), 'type': 's', 'value': data};
        else:
            self.frame['data']['data'] = {'size': 0, 'type': 's', 'value': ''};

        # the checksum is calculated as for the ipv4 datagram, but considering 
        # the whole datagaram
        self.frame['header']['cksum']['value'] = self.get_cksum()

    def get_cksum(self):

        """the checksum field of an icmp packet is used by receivers to quickly 
        verify its integrity against network errors (not malicious attacks).

        it is defined as the 16 bit 1's complement of the 1's complement 
        sum of all 16 bit words in the header AND data. for purposes of 
        computing the checksum, the value of the checksum field is zero."""

        cksum = 0
        # get a byte array representation of the icmp packet
        icmp_pckt = self.pack(parts = ['header', 'data'])
        # keep summing blocks of 2 byte from icmp_pckt to get a 32 bit sum
        count = len(icmp_pckt)
        while (count > 1):
            # the 16 bit sum is done in 2 parts:
            #   - isolate 2 bytes in icmp_pckt
            #   - use int(<pair>, 16) to convert it to a numerical value
            cksum += int(icmp_pckt[(count - 2):(count - 1)], 16)
            count -= 2

        # add left-over byte, if any
        if count > 0:
            # note how we use int(, 8) to convert it to a numerical value
            cksum += int(icmp_pckt[0], 8)

        # fold result into a 16 bit 1's complement sum
        while (cksum >> 16):
            cksum = (cksum & 0xFFFF) + (cksum >> 16)

        # return the complement of the 16 bit 1's complement sum. done.
        return ~cksum

class ICMP_Module:

    def __init__(self, stack):

        # reference to the stack the icmp object belongs to
        self.stack  = stack

    def process_pckt(self, ipv4_dgram):

        # unpack the ipv4 payload into an ICMP packet object, extract the header
        # fields
        icmp_pckt = ICMP_Packet()
        icmp_pckt.unpack_hdr(ipv4_dgram.get_attr('data', 'data'))

        # process the icmp packet according to the 'type' field. we only 
        # support: ECHO_REQUEST and DESTINATION_UNREACHABLE
        if icmp_pckt.get_attr('header', 'type') == ICMP_Packet.ICMP_TYPE_ECHO_REQUEST:

            # the icmp header info to send in the reply differs in the 'type' field
            icmp_pckt.set_attr('header', 'type', ICMP_Packet.ICMP_TYPE_ECHO_REPLY)
            icmp_pckt.set_attr('header', 'code', 0)
            icmp_pckt.set_attr('header', 'cksum', self.get_cksum())

            # encapsulate icmp packet within an ipv4 packet and send it back
            # to the source
            self.stack.ipv4_mod.send_dgram(
                src_ip  = self.stack.ip,
                dst_ip  = ipv4_dgram.get_attr('header', 'saddr'),
                payload = icmp_pckt.pack())

        else:
            print("ICMP::process_pckt() [ERROR] unknown icmp type : %02x" % (icmp_pckt.get_attr('header', 'type')))
