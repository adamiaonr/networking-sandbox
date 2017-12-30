import os 
import sys
import struct
import subprocess
import ipaddress
import arch
import socket

# from pytun import TunTapDevice, IFF_TAP, IFF_NO_PI
# pytun alternative: http://www.secdev.org/projects/tuntap_udp/files/tunproxy.py 
from fcntl import ioctl
# custom import
from ethernet import Ethernet

# ioctl constants
TUNSETIFF = 0x400454ca
TUNSETPERSIST = 0x400454cb

IFF_TUN = 0x0001
IFF_TAP = 0x0002
IFF_NO_PI = 0x1000

SIOCGIFHWADDR = 0x00008927

def run_cmd(cmd, wait = False):

    try:

        p = subprocess.Popen(
            [cmd], 
            stdout = subprocess.PIPE,
            shell = True)
        p.wait()

        (result, error) = p.communicate()
        
    except subprocess.CalledProcessError as e:
        sys.stderr.write(
            "%s::run_cmd() : [ERROR]: output = %s, error code = %s\n"
            % (sys.argv[0], e.output, e.returncode))

    return p.returncode, result, error

class Tap:

    def __init__(self, 
        net_addr = '10.0.0.4', 
        net_mask = '255.255.255.0', 
        hw_addr = '', 
        mtu = 1500):

        # create a tap device to send/receive L2 frames
        self.net_addr = net_addr
        self.net_mask = net_mask
        self.mtu = mtu
        self.hw_addr = hw_addr
        # tap device name
        # FIXME: now hardcoded, try to fix this
        self.dev_name = 'tap0'

        try:
            # open tap device
            tap_fd = os.open('/dev/net/tun', os.O_RDWR)
            
            # set tap device flags via ioctl():
            #
            # IFF_TUN   : tun device (no Ethernet headers)
            # IFF_TAP   : tap device
            # IFF_NO_PI : do not provide packet information, otherwise we end 
            #             up with unnecessary packet information prepended to 
            #             the Ethernet frame
            ifr = struct.pack("16sH", ("%s" % (self.dev_name)), IFF_TAP | IFF_NO_PI)
            ioctl(tap_fd, TUNSETIFF, ifr)

            # # set device to persistent
            # ioctl(tap_fd, TUNSETPERSIST, 1)

            print("tap::init() : [INFO] tap device w/ name %s allocated" % (ifr[:16].strip("\x00")))
            
            # bring device up and set route for its net address
            self.up()

        except Exception as e:
            print("tap::init() : [ERROR] cannot setup tap device (%s)" % (e.message))

        self.fd = tap_fd
        self.hw_addr = self.get_mac()

    def write(self, data):
        os.write(self.fd, data)

    def read(self, mtu = 1500):
        return os.read(self.fd, self.mtu)

    def get_mac(self):
        ifr = struct.pack('16sH14s', self.dev_name, socket.AF_UNIX, '\x00' * 14)
        res = ioctl(self.fd, SIOCGIFHWADDR, ifr)
        address = struct.unpack('16sH14s', res)[2]
        mac = struct.unpack('6B8x', address)
        return ":".join(['%02X' % i for i in mac])

    def up(self):

        # bring tap device up
        if run_cmd('ip link set dev %s up' % (self.dev_name), wait = True)[0]:
            print("tap::up() : [ERROR] error setting up %s tap device" % (self.dev_name))
            return 1

        # add route for tap device net address
        cidr = str(ipaddress.IPv4Network((unicode(".".join(self.net_addr.split(".")[:-1]) + ".0"), unicode(self.net_mask))))
        if run_cmd('ip route add dev %s %s' % (self.dev_name, cidr), wait = True)[0]:
            print("tap::up() : [ERROR] error route for %s tap device (%s)" % (self.dev_name, cidr))
            return 1

        return 0

    def shutdown(self):

        # bring tap device down
        if run_cmd('ip link delete %s' % (self.dev_name), wait = True)[0]:
            print("tap::up() : [ERROR] error bringing down %s tap device" % (self.dev_name))

        self.fd.close()
        return 0

    def print_info(self):

        print("tap::print_info() : [INFO] tap device info:")
        print("\tname: %s" % (self.dev_name))
        print("\taddr: %s" % (self.net_addr))
        print("\tnetmask: %s" % (self.net_mask))
        print("\thwaddr: %s (%s)" % (self.hw_addr, type(self.hw_addr)))
        print("\tmtu: %s" % (self.mtu))