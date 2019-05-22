import struct
import binascii
import ipaddress

from ethernet import Ethernet, mac_to_str
from metaframe import MetaFrame
from collections import defaultdict
from prettytable import PrettyTable


# modeled in the style of Linux's routing table ('route -n')
class Route_Entry:

    # routing entry flags (as in Linux routing table)
    RT_LOOPBACK = 0x01  # route to self (?)
    RT_GATEWAY = 0x02   # route is to a gw. if not present, one can assume the route is to a directly connected dst.
    RT_HOST = 0x04      # route to a host (/32). if not present, one can assume this is a route to a network
    RT_REJECT = 0x08    # ???
    RT_UP = 0x10        # route is up
    
    
    def __init__(self, dst, gw, netmask, flags, iface):
        
        self.dst = dst
        self.gw = gw
        self.netmask = netmask
        self.flags = flags
        self.iface = iface
        
    def print_flags(self):
        
        rt_str = ''

        if self.flags & Route_Entry.RT_UP:
            rt_str += 'U'

        if self.flags & Route_Entry.RT_LOOPBACK:
            rt_str += 'L'
            
        if self.flags & Route_Entry.RT_GATEWAY:
            rt_str += 'G'

        if self.flags & Route_Entry.RT_HOST:
            rt_str += 'H'

        if self.flags & Route_Entry.RT_REJECT:
            rt_str += 'R'
            
        return rt_str

class Route_Module:

    def __init__(self, stack):

        self.stack = stack

        # routing table
        self.route_table = set([])

    def print_table(self):

        table = PrettyTable(['dst', 'gw', 'netmask', 'flags', 'iface'])
        
        for rt_entry in self.route_table:
            table.add_row([
                ('%s' % (str(ipaddress.IPv4Address(rt_entry.dst)))),
                ('%s' % (str(ipaddress.IPv4Address(rt_entry.gw)))), 
                ('%s' % (str(ipaddress.IPv4Address(rt_entry.netmask)))),
                ('%s' % (rt_entry.print_flags())),
                ('%s' % (rt_entry.iface))
                ])
    
        print('python-ip-stack IP routing table')
        print(table)

    def initialize(self):
        # gw :: dst : 0.0.0.0, gw : 10.0.0.5, netmask : 0.0.0.0, flags : UG, iface : tap0 
        self.add_route(0, int(ipaddress.IPv4Address(unicode(self.stack.tap.net_addr))), 0, Route_Entry.RT_GATEWAY, self.stack.tap.dev_name)

    def add_route(self, dst, gw, netmask, flags, iface):
        self.route_table.add(Route_Entry(dst, gw, netmask, flags, iface))
        
    def lookup(self, dst_ip):
        
        for rt_entry in self.route_table:
            if ((rt_entry.dst & rt_entry.netmask) & (dst_ip & rt_entry.netmask)):
                break
            
        return rt_entry
                