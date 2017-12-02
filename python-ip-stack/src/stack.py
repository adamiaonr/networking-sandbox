import array
import time
import struct
import binascii
import ipaddress
import thread

from socket import *

from tap import Tap
from ethernet import Ethernet
from arp import ARP_Module
from ipv4 import IPv4_Module, IPv4_Dgram
from icmp import ICMP_Module

class Stack:

    def __init__(self, net_addr = '10.0.0.4'):

        # tap device used by stack to send/receive packets
        self.tap = Tap(net_addr)
        # local mac and ip
        self.mac = self.tap.dev.hwaddr
        self.ip = int(ipaddress.IPv4Address(unicode(self.tap.dev.addr)))

        # initialize modules for supported protocols:
        #   - ARP : maintains local ARP table and processes ARP requests
        self.arp_mod = ARP_Module(self)

        #   - IPv4 : handles IP datagrams, 'unmarshalling' packets coming in 
        #            from the network (passing them to upper layer processing 
        #            daemons), marshalling data coming from upper layer 
        #            protocol daemons
        self.ipv4_mod = IPv4_Module(self)

        #   - ICMP : handles ICMP protocol packets (mainly ping responses)
        self.icmp_mod = ICMP_Module(self)

        # hack : icmp listener to handle icmp packets (Linux doesn't let me 
        # receive icmp packets with a python tap device)
        self.icmp_listener_sock = None

    def send_frame(self, frame_type, dst_mac, data):

        if frame_type in [Ethernet.PROTO_ARP, Ethernet.PROTO_IPv4]:

            eth_frame = Ethernet(
                src_mac      = self.tap.dev.hwaddr,
                dst_mac      = dst_mac,
                eth_type     = frame_type,
                payload      = data,
                payload_size = len(data))

            self.tap.dev.write(eth_frame.pack())

        else:
            print("Stack::send_frame() : [ERROR] unknown frame type : %d" % (frame_type))

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

        else:
            print("Stack::handle_frame() [ERROR] unknown protocol type : %02x" % (frame_type))

def start_icmp_listener(stack):

    # hack : i've found it hard to re-direct icmp traffic to the tap 
    # interface. to circumvent this problem, i try using a raw socket, 
    # w/ protocol option 'IPPROTO_ICMP'
    stack.icmp_listener_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)

    # bind the socket to the tap interface if the python stack
    stack.icmp_listener_sock.bind((stack.tap.dev.addr, 0))

    # recv loop
    (raw_dgram, snd_addr) = stack.icmp_listener_sock.recvfrom(2048)

    # process icmp packet (since proto is IPPROTO_ICMP, we know for sure that 
    # the received packet will be icmp)
    icmp_ipv4_dgram = IPv4_Dgram()
    icmp_ipv4_dgram.unpack(raw_dgram)
    stack.icmp_mod.process_pckt(icmp_ipv4_dgram)

if __name__ == "__main__":

    # initialize a python tcp/ip stack
    stack = Stack('10.0.0.4')
    stack.tap.print_info()

    # hack : start icmp listener thread, to overcome the limitations of icmp 
    # handling in Linux's networking Stack
    try:
        thread.start_new_thread(start_icmp_listener, (stack,))
    except:
        print("error while starting thread...")

    steps = 100
    while (steps > 0):
        frame = stack.tap.dev.read(stack.tap.dev.mtu)
        stack.handle_frame(frame)
        
        steps -= 1

    stack.tap.shutdown()

    print("exiting...")