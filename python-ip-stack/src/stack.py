import binascii
import ipaddress

from tap import Tap
from ethernet import Ethernet
from arp import ARP_Module
from ipv4 import IPv4_Module
from icmp import ICMP_Module
from pytransport import PyTransport

class Stack:

    def __init__(self, net_addr = '10.0.0.4'):

        # tap device used by stack to send/receive packets
        self.tap = Tap(net_addr)
        # local mac and ip
        self.mac = self.tap.hw_addr
        self.ip = int(ipaddress.IPv4Address(unicode(self.tap.net_addr)))

        # initialize (singleton) modules for supported protocols:

        #   - ARP : maintains local ARP table and processes ARP requests
        self.arp_mod = ARP_Module(self)

        #   - IPv4 : handles IP datagrams, 'unmarshalling' packets coming in 
        #            from the network (passing them to upper layer processing 
        #            daemons), marshalling data coming from upper layer 
        #            protocol daemons
        self.ipv4_mod = IPv4_Module(self)

        #   - ICMP : handles ICMP protocol packets (mainly ping responses)
        self.icmp_mod = ICMP_Module(self)

        #   - PyTransport : handles everything related to tcp/udp
        self.transport_mod = PyTransport(self)

    def send_frame(self, frame_type, dst_mac, data):

        if frame_type in [Ethernet.PROTO_ARP, Ethernet.PROTO_IPv4]:

            eth_frame = Ethernet(
                src_mac      = binascii.unhexlify(self.mac.replace(':', '')),
                dst_mac      = dst_mac,
                eth_type     = frame_type,
                payload      = data,
                payload_size = len(data))

            self.tap.write(eth_frame.pack())

        else:
            print("stack::send_frame() : [ERROR] unknown frame type : %d" % (frame_type))

    def handle_frame(self, raw_frame):

        # fill Ethernet object from the wire using unpack_hdr(). this will 
        # only fill the header
        eth_frame = Ethernet()
        eth_frame.unpack_hdr(raw_frame)

        # process Ethernet payload according to eth_type
        frame_type = eth_frame.get_attr('header', 'eth_type')
        
        # ARP frame
        if frame_type == Ethernet.PROTO_ARP:
            eth_frame.unpack_data(raw_frame)
            self.arp_mod.process_dgram(eth_frame.get_attr('data', 'payload'))

        # IPv4 datagram
        elif frame_type == Ethernet.PROTO_IPv4:

            eth_frame.unpack_data(raw_frame)
            self.ipv4_mod.process_dgram(eth_frame.get_attr('data', 'payload'))

        # else:
        #     print("Stack::handle_frame() [ERROR] unknown protocol type : %02x" % (frame_type))

if __name__ == "__main__":

    # initialize a python tcp/ip stack
    stack = Stack('10.0.0.4')
    stack.tap.print_info()

    steps = 100
    while (steps > 0):
        frame = stack.tap.read()
        stack.handle_frame(frame)
        
        steps -= 1

    stack.tap.shutdown()

    print("exiting...")