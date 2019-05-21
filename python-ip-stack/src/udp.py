from ethernet import Ethernet
from metaframe import MetaFrame
from collections import defaultdict

class UDP_Dgram(MetaFrame):

    # udp header is 8 byte long:
    #   -# 16 bit src port number
    #   -# 16 bit dst port number
    #   -# 16 bit udp length (header + data)
    #   -# 16 bit checksum (covers (pseudo) header + data)
    UDP_HDR_SIZE = 0x08
    UDP_FTR_SIZE = 0x00

    def __init__(self, 
        src_port = 0x00, 
        dst_port = 0x00,
        udp_data = None, udp_data_size = 0,
        src_ip = 0, dst_ip = 0, proto = 0x11):

        MetaFrame.__init__(self)

        self.hdr_size = UDP_Dgram.UDP_HDR_SIZE
        self.ftr_size = UDP_Dgram.UDP_FTR_SIZE

        self.pseudo_header = defaultdict()
        self.pseudo_header['src_ip'] = src_ip
        self.pseudo_header['dst_ip'] = dst_ip
        self.pseudo_header['proto'] = proto

        self.frame['header']['src_port']    = {'size': 1, 'type': 'H', 'value': 0x0000}
        self.frame['header']['dst_port']    = {'size': 1, 'type': 'H', 'value': 0x0000}
        self.frame['header']['length']      = {'size': 1, 'type': 'H', 'value': 0x0000}

        if udp_data:
            data = udp_data.pack()
            self.frame['data']['data'] = {'size': len(data), 'type': 's', 'value': data};
        else:
            self.frame['data']['data'] = {'size': 0, 'type': 's', 'value': ''};

        # the checksum is calculated as for the ipv4 datagram, but considering 
        # the whole datagram + pseudo-header
        self.frame['header']['cksum']['value'] = self.get_cksum()

    def get_cksum(self):

        """the checksum field of an udp datagram is used by receivers to quickly 
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
