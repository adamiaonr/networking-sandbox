from ethernet import Ethernet
from pytun import TunTapDevice, IFF_TAP, IFF_NO_PI

class Tap:

    def __init__(self, 
        net_addr = '127.0.0.1', net_mask = '255.255.255.0', 
        hw_addr = '', mtu = 1500):

        # create a tap device to send/receive L2 frames
        tap_dev = TunTapDevice(flags = (IFF_TAP | IFF_NO_PI))
        tap_dev.persist(True)
        # set TunTapDevice parameters
        tap_dev.addr    = net_addr
        tap_dev.netmask = net_mask
        # tap_dev.hwaddr  = hw_addr
        tap_dev.mtu = mtu
        # set tap device UP
        tap_dev.up()

        # set tap_dev as the stack's device: all network i/o will 
        # use this device from now on
        self.dev = tap_dev

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