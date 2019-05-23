import sys
import binascii
import ipaddress
import argparse

from tap import Tap
from ethernet import Ethernet
from arp import ARP_Module
from ipv4 import IPv4_Module
from route import Route_Module
from icmp import ICMP_Module
from pytransport import PyTransport

class Stack:

    def __init__(self, tap_addr, node_mac_addr, node_ip_addr):

        # tap device used by stack to send/receive packets
        self.tap = Tap(tap_addr)
        # local mac and ip
        self.mac_addr = node_mac_addr
        self.ip_addr = ipaddress.IPv4Address(unicode(node_ip_addr))

        # initialize (singleton) modules for supported protocols:

        #   - ARP : maintains local ARP table and processes ARP requests
        self.arp_mod = ARP_Module(self)

        #   - IPv4 : handles IP datagrams, 'unmarshalling' packets coming in 
        #            from the network (passing them to upper layer processing 
        #            daemons), marshalling data coming from upper layer 
        #            protocol daemons
        self.ipv4_mod = IPv4_Module(self)
        
        #   - route : stack's routing module
        self.route_mod = Route_Module(self)
        self.route_mod.initialize()
        self.route_mod.print_table()

        #   - ICMP : handles ICMP protocol packets (mainly ping responses)
        self.icmp_mod = ICMP_Module(self)

        #   - PyTransport : handles everything related to tcp/udp
        self.transport_mod = PyTransport(self)

    def send_frame(self, frame_type, dst_mac, data):

        if frame_type in [Ethernet.PROTO_ARP, Ethernet.PROTO_IPv4]:

            eth_frame = Ethernet(
                src_mac      = binascii.unhexlify(self.mac_addr.replace(':', '')),
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
    
    # use an ArgumentParser for a nice CLI
    parser = argparse.ArgumentParser()

    # options (self-explanatory)
    parser.add_argument(
        "--tap-addr", 
         help = """ip addr & netmask of tap interface in cidr notation. 
         default : 10.0.0.1/24""")

    parser.add_argument(
        "--node-mac-addr", 
         help = """mac addr of 'virtual' network node. 
         default : 00:0c:29:6d:50:25""")    

    parser.add_argument(
        "--node-ip-addr", 
         help = """ip addr of 'virtual' network node. 
         must be in tap-addr subnet.
         e.g., if tap-addr is 10.0.0.1/24, node-addr should be 10.0.0.[2-254].
         default : 10.0.0.4""")

    args = parser.parse_args()

    if not args.tap_addr:
        args.tap_addr = '10.0.0.1/24'

    if not args.node_mac_addr:
        args.node_mac_addr = '01:23:45:67:89:ab'

    if not args.node_ip_addr:
        args.node_ip_addr = '10.0.0.4'
        
    if ipaddress.IPv4Address(unicode(args.node_ip_addr)) not in ipaddress.IPv4Network(unicode(args.tap_addr), strict = False):
        sys.stderr.write("""%s: [ERROR] node ip addr not in tap subnet\n""" % sys.argv[0]) 
        parser.print_help()
        sys.exit(1)

    # initialize a python tcp/ip stack
    stack = Stack(tap_addr = args.tap_addr, node_mac_addr = args.node_mac_addr, node_ip_addr = args.node_ip_addr)
    stack.tap.print_info()

    steps = 100
    while (steps > 0):
        frame = stack.tap.read()
        stack.handle_frame(frame)
        
        steps -= 1

    stack.tap.shutdown()

    print("exiting...")