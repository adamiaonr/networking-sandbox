import struct
import binascii
import collections

from metaframe import MetaFrame

# from scapy.all import * # in order to understand how packets are formed, it 
#                         # can be useful to use your own Ethernet lib to create 
#                         # Ethernet packets

# ETHERNET_PREAMBLE = '\x55\x55\x55\x55\x55\x55\x55'
# ETHERNET_SFD = 171

class Ethernet(MetaFrame):

    ETH_HDR_SIZE = 0x0E
    ETH_FCS_SIZE = 0x04

    PROTO_ARP   = 0x0806
    PROTO_IPv4  = 0x0800

    def __init__(self, 
        src_mac = '', dst_mac = '',
        eth_type = 0x002E,
        payload = ('0' * 46), payload_size = 46):

        MetaFrame.__init__(self)

        self.hdr_size = Ethernet.ETH_HDR_SIZE
        self.ftr_size = Ethernet.ETH_FCS_SIZE

        # FIXME: apparently, adding a preamble and sfd isn't necessary. somehow 
        # these fields are automatically added to the Ethernet header below 
        # when sent over a pytun tap device.
        # self.frame['preamble']  = {'size': 7, 'type': 's', 'value': ETHERNET_PREAMBLE}
        # self.frame['sfd']       = {'size': 1, 'type': 'B', 'value': ETHERNET_SFD}
        self.frame['header']['dst_mac']   = {'size': 6, 'type': 's', 'value': dst_mac}
        self.frame['header']['src_mac']   = {'size': 6, 'type': 's', 'value': src_mac}
        self.frame['header']['eth_type']  = {'size': 1, 'type': 'H', 'value': eth_type};
        self.frame['data']['payload']   = {'size': payload_size, 'type': 's', 'value': payload}
        self.frame['footer']['fcs']       = {'size': 4, 'type': 's', 'value': struct.pack("I", self.calc_fcs())}

    # def __str__(self):
    #     return (""" %s, %s""" % (self., self., self.))

    def calc_fcs(self):

        """ 4 octet (32 bit) CRC field, computed over the following fields:
                -# src_mac and dst_mac
                -# eth_type
                -# payload
                -# pad : pad is necessary if the payload is less than 46 byte 
                         (making the Ethernet frame less than 64 byte)

            the crc 32 calculation for Ethernet uses the polynomial 0x104C11DB7, 
            and a specific sequence of steps which differ from typical crc 
            calculation. this implementation was based on info gathered from:
                -# http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html#ch6
                -# http://stackoverflow.com/questions/9286631/ethernet-crc32-calculation-software-vs-algorithmic-result
                -# http://stackoverflow.com/questions/5047494/python-crc-32-woes

            tested this successfuly with wireshark.
        """

        # crc32 polynomial specified in IEEE 802.3 standard : 0x104C11DB7
        # (https://users.ece.cmu.edu/~koopman/networks/dsn02/dsn02_koopman.pdf)
        # the MSB is omitted : note that the bit size of a polynomial for a 
        # n bit crc is n + 1
        polynomial = 0x04C11DB7

        # pass the message to a byte stream. this accomplished in 2 steps:
        #   1) convert the Ethernet frame's fields to a byte stream using struct.pack(). 
        #      note that this byte stream follows network byte order (big endian)
        #   2) each byte after step 1 is represented as a char. convert each to 
        #      numerical form (using ord()) suitable for bitwise operations
        frame_bytes = [ord(byte_str) for byte_str in self.pack(field_exceptions = ['fcs'])]

        # now comes the weird part. crc 32 (as used by Ethernet) is slightly 
        # different from the typical crc calculations.

        # 1) reverse the order of the bits for each byte in the input message
        #    based on http://stackoverflow.com/questions/12681945/reversing-bits-of-python-integer
        frame_bytes = [int('{:08b}'.format(b)[::-1], 2) for b in frame_bytes]
        # 2) invert the bits in crc (initially set to 0x00000000)
        crc = 0x00000000 ^ 0xFFFFFFFF

        # 3) now do the typical crc calculation (shif register approach)
        for byte in frame_bytes:

            # xor the current byte of the stream
            crc = (crc ^ (byte << 24)) & 0xFFFFFFFF
            # calculate crc on the 
            for i in xrange(8):

                if ((crc & 0x80000000) != 0):
                    crc = ((crc << 1) ^ polynomial) & 0xFFFFFFFF
                else:
                    crc = (crc << 1) & 0xFFFFFFFF

        # 4) complement the crc again
        crc = crc ^ 0xFFFFFFFF
        # 5) finally, reverse the bits of the crc
        b = '{:0{width}b}'.format(crc, width = 32)
        crc = int(b[::-1], 2)

        return crc
        