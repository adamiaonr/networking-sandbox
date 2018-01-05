import struct
import binascii
import collections
import ipaddress

from pytransport import PyTransport
from arp import ARP_Module, ARP_Dgram
from ethernet import Ethernet
from metaframe import MetaFrame
from collections import defaultdict

class IPv4_Dgram(MetaFrame):

	# ipv4 header is 20 byte 
    IPv4_HDR_SIZE = 0x05
    IPv4_FTR_SIZE = 0x00

    IPv4_FLAG_DF  = 0x02
    IPv4_FLAG_MF  = 0x01

    IPv4_PROTO_ICMP = 0x01
    IPv4_PROTO_TCP = 0x06
    IPv4_PROTO_UDP = 0x11

    def __init__(self, 
        tos = 0x00, 
        dgram_id = 0x0000,
        flags = 0x00,
        frag_offset = 0x0000,
        ttl = 0xFF,
        proto = 0x10,
        saddr = 0, daddr = 0,
        data = ''):

        MetaFrame.__init__(self)

        self.hdr_size = ((IPv4_Dgram.IPv4_HDR_SIZE * 32) / 8)
        self.ftr_size = IPv4_Dgram.IPv4_FTR_SIZE

        # FIXME: for now, let's keep the default values for version (4 for ipv4) 
        # and default ihl (internet header length) (20 byte). the 4 most significant 
        # bits are for version, 4 LSBs to ihl.
        self.frame['header']['version & ihl'] = {'size': 1, 'type': 'B', 'value': (0x40 | IPv4_Dgram.IPv4_HDR_SIZE)}
        # type of service (tos)
        self.frame['header']['tos'] = {'size': 1, 'type': 'B', 'value': tos}
        # total length of ip datagram (including payload). 16 bit.
        dgram_len = ((IPv4_Dgram.IPv4_HDR_SIZE * 32) / 8) + len(data)
        self.frame['header']['len'] = {'size': 1, 'type': 'H', 'value': dgram_len};
        # id of datagram, used for re-assembly of fragmented ipv4 datagrams.
        self.frame['header']['id'] = {'size': 1, 'type': 'H', 'value': dgram_id};
        # flags (3 MSBs):
        #	-# bit 0: always 0 (reserved)
        #	-# bit 1: don't fragment (DF), instructs not to fragment datagram
        #	-# bit 2: more fragments (MF), for all datagrams, except the last	
        #
        # remaining 13 bits is the fragment offset, i.e. the position of the 
        # fragment in a datagram
        self.frame['header']['flags & frag_offset'] = {'size': 1, 'type': 'H', 'value': ((flags << 1) | frag_offset)};
        # time to live (ttl)
        self.frame['header']['ttl'] = {'size': 1, 'type': 'B', 'value': ttl};
        # proto, the protocol carried within the data portion of the datagram
        self.frame['header']['proto'] = {'size': 1, 'type': 'B', 'value': proto};
        # cksum, the internet checksum field, used to verify the integrity of 
        # ipv4 header. initially set to 0x0000.
        self.frame['header']['cksum'] = {'size': 1, 'type': 'H', 'value': 0x0000};
        # source and destinatin ipv4 addresses (32 bit), in that order
        self.frame['header']['saddr'] = {'size': 1, 'type': 'I', 'value': saddr};
        self.frame['header']['daddr'] = {'size': 1, 'type': 'I', 'value': daddr};

        # update the checksum value
        self.frame['header']['cksum']['value'] = self.get_cksum()

        # finally, the payload
        self.frame['data']['data'] = {'size': len(data), 'type': 's', 'value': data}

    def get_cksum(self, include_data = False):

        """the checksum field of an ipv4 header is used by receivers of ipv4 
        datagrams to quickly verify the integrity of the header (against 
        network errors, not malicious attacks).

        it is defined as the 16 bit one's complement of the one's complement 
        sum of all 16 bit words in the header. for purposes of computing the 
        checksum, the value of the checksum field is zero."""

        cksum = 0
        # convert the header to an array of bytes
        if include_data:
            ipv4_dgram = self.pack(parts = ['header', 'data'])
        else:
            ipv4_dgram = self.pack(parts = ['header'])

        count = len(ipv4_dgram)
        # transform each byte in ipv4_dgram to a number in [0, 255]
        dgram_bytes = [ord(byte_str) for byte_str in ipv4_dgram]
        # keep summing blocks of 2 byte from ipv4_dgram to get a 16 bit sum
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

class IPv4_Module:

    def __init__(self, stack):
        self.stack = stack

    def process_dgram(self, raw_dgram):

        # read raw ipv4 dgram into IPv4_Dgram object
        ipv4_dgram = IPv4_Dgram()
        ipv4_dgram.unpack(raw_dgram)

        # pass the payload of ip datagram to the appropriate protocol 
        # handler
        if ipv4_dgram.get_attr('header', 'proto') == IPv4_Dgram.IPv4_PROTO_ICMP:
            self.stack.icmp_mod.process_pckt(ipv4_dgram)

        elif ipv4_dgram.get_attr('header', 'proto') == IPv4_Dgram.IPv4_PROTO_UDP:
            self.stack.transport_mod.process_dgram(ipv4_dgram)

        # else:
        #     print("ipv4::process_dgram() [ERROR] unknown protocol type : %02x" % (ipv4_dgram.get_attr('header', 'proto')))

    def send_dgram(self, src_ip, dst_ip, proto, payload = ''):

        # prepare ipv4 dgram to send
        # FIXME : not how we used a LOT of default values for the IPv4 header, 
        # whose impact has not been tested
        ipv4_dgram = IPv4_Dgram(
            proto   = proto,
            saddr   = src_ip, 
            daddr   = dst_ip,
            data    = payload)

        # encapsulate it in a Ethernet frame and send it
        # get dmac using ARP (you see? this is the stack workin'...)
        arp_entry = self.stack.arp_mod.get_record(ARP_Dgram.ARP_PROTYPE_IPv4, dst_ip)
        if arp_entry is None:
            print("ipv4::send_dgram() [ERROR] no mac address on ARP table for dst ip : %s" % (inet_ntoa(struct.pack('!L', dst_ip))))
            # FIXME: we should send an ARP request and update the ARP table now
            return -1

        self.stack.send_frame(Ethernet.PROTO_IPv4, arp_entry.smac, ipv4_dgram.pack())
