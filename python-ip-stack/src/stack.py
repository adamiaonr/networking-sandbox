import array
import time
import struct
import binascii

from tap import Tap
from ethernet import Ethernet
from arp import ARP_Daemon

class Stack:

    def __init__(self, net_addr = '10.0.0.4'):

        self.tap = Tap(net_addr)
        self.arp_daemon = ARP_Daemon(self.tap)

    def handle_frame(self, raw_frame):

        # create Ethernet object from wire format
        eth_frame = Ethernet()
        eth_frame.unpack_hdr(raw_frame)

        frame_type = eth_frame.get_attr('header', 'eth_type')
        if frame_type == Ethernet.PROTO_ARP:

            # if frame encapsulates an ARP packet, pass the 
            # payload to the ARP daemon
            eth_frame.unpack_data(raw_frame)
            self.arp_daemon.process_arp_packet(eth_frame.get_attr('data', 'payload'))

        else:
            print("stack::handle_frame() [ERROR] unknown protocol type : %02x" % (frame_type))

if __name__ == "__main__":

    # init a python tcp/ip stack
    stack = Stack('10.0.0.4')
    stack.tap.print_info()

    steps = 100
    while (steps > 0):
        frame = stack.tap.dev.read(stack.tap.dev.mtu)
        stack.handle_frame(frame)
        
        steps -= 1

    stack.tap.shutdown()

    print("exiting...")