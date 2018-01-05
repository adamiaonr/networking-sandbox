import struct
import binascii
import collections
import ipaddress

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

        cksum = 0x0000
        # get a byte array representation of the icmp packet
        icmp_pckt = self.pack(parts = ['header', 'data'])
        # keep summing blocks of 2 byte from icmp_pckt to get a 32 bit sum
        count = len(icmp_pckt)
        # transform each byte in icmp packet to a number in [0, 255]
        dgram_bytes = [ord(byte_str) for byte_str in icmp_pckt]

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

class ICMP_Module:

    def __init__(self, stack):
        self.stack = stack

    def process_pckt(self, ipv4_dgram):

        # unpack the ipv4 payload into an ICMP packet object, extract the header
        # fields
        icmp_pckt = ICMP_Packet()
        icmp_pckt.unpack(ipv4_dgram.get_attr('data', 'data'))

        # process the icmp packet according to the 'type' field. we only 
        # support: ECHO_REQUEST and DESTINATION_UNREACHABLE

        if icmp_pckt.get_attr('header', 'type') == ICMP_Packet.ICMP_TYPE_ECHO_REQUEST:

            # the icmp header info to send in the reply differs in the 'type' field
            icmp_pckt.set_attr('header', 'type', ICMP_Packet.ICMP_TYPE_ECHO_REPLY)
            icmp_pckt.set_attr('header', 'code', 0)
            # set the data field to be equal to the data sent in the echo request
            icmp_pckt.set_attr('data', 'data', icmp_pckt.get_attr('data', 'data'), size = len(icmp_pckt.get_attr('data', 'data')))
            # calculate the checksum
            icmp_pckt.set_attr('header', 'cksum', 0x0000)
            icmp_pckt.set_attr('header', 'cksum', icmp_pckt.get_cksum())

            # encapsulate icmp packet within an ipv4 packet and send it back
            # to the source
            self.stack.ipv4_mod.send_dgram(
                src_ip  = self.stack.ip,
                dst_ip  = ipv4_dgram.get_attr('header', 'saddr'),
                proto = ipv4_dgram.get_attr('header', 'proto'),
                payload = icmp_pckt.pack())

        else:
            print("icmp::process_pckt() [ERROR] unknown icmp type : %02x" % (icmp_pckt.get_attr('header', 'type')))
