from ethernet import Ethernet
from pytun import TunTapDevice, IFF_TAP, IFF_NO_PI

class Tap:

    def __init__(self, 
        net_addr = '127.0.0.1', net_mask = '255.255.255.0', 
        hw_addr = '', mtu = 1500):

        # create a tap device to send/receive L2 frames
        tap_dev = TunTapDevice(flags = (IFF_TAP | IFF_NO_PI))
        tap_dev.persist(True)
        # set parameters
        tap_dev.addr    = net_addr
        tap_dev.netmask = net_mask
        # tap_dev.hwaddr  = hw_addr
        tap_dev.mtu = mtu
        # set tap device UP
        tap_dev.up()

        self.dev = tap_dev

    def send_frame(self, frame_type, dst_mac, data):

        if frame_type == Ethernet.PROTO_ARP:

            eth_frame = Ethernet(
                src_mac      = self.dev.hwaddr,
                dst_mac      = dst_mac,
                eth_type     = frame_type,
                payload      = data,
                payload_size = len(data))

            self.dev.write(eth_frame.pack())

        else:
            print("Tap::send() : [ERROR] unknown frame type : %d" % (frame_type))

    def shutdown(self):
        self.dev.down()
        self.dev.close()

    def print_info(self):

        print("Tap::print_tap_dev_info() : [INFO] tap device info:")
        print("\tname: %s" % (self.dev.name))
        print("\taddr: %s" % (self.dev.addr))
        print("\tnetmask: %s" % (self.dev.netmask))
        print("\thwaddr: %s (%s)" % (self.dev.hwaddr, type(self.dev.hwaddr)))
        print("\tmtu: %s" % (self.dev.mtu))