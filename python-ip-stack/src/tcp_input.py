import ipaddress

from tcp import TCP_Flags
from tcp import TCP_Seg
from ipv4 import IPv4_Dgram

# TCP flag bitmasks
# FIXME : this should be in TCP_Flags() ?
NS  = 0x100
CWR = 0x080
ECE = 0x040
URG = 0x020
ACK = 0x010
PSH = 0x008
RST = 0x004
SYN = 0x002
FIN = 0x001

# TCP connection states
LISTEN = 0x01
SYN_RECEIVED = 0x03
ESTABLISHED = 0x04

class TCP_Input_Module:

    def __init__(self, stack):
        self.stack = stack
        self.state = LISTEN

    def process_tcp_seg(self, ipv4_dgram):

        tcp_seg_in = TCP_Seg()
        tcp_seg_in.unpack(ipv4_dgram.get_attr('data', 'data'))

        # extract tcp flags
        seq_nr_in = tcp_seg_in.get_attr('header', 'seq_number')
        ack_nr_in = tcp_seg_in.get_attr('header', 'ack_number')
        tcp_flags_in = (tcp_seg_in.get_attr('header', 'hdr_length_flags') & 0x01FF)

        # FIXME : debugging
        print("tcp_input::process_tcp_seg() [INFO] received tcp seg : ")
        tcp_seg_in.pseudo_header['src_addr']['value'] = ipv4_dgram.get_attr('header', 'saddr')
        tcp_seg_in.pseudo_header['dst_addr']['value'] = ipv4_dgram.get_attr('header', 'daddr')        
        print(str(tcp_seg_in))
        print('tcp flags: %s' % (str(TCP_Flags(flags_hex = tcp_flags_in))))

        if (self.state == LISTEN) and ((tcp_flags_in & 0x01FF) & SYN):
            # SYN received : update state and answer back w/ a SYN-ACK
            self.state = SYN_RECEIVED
            # prepare pseudo header attributes
            src_addr = ipv4_dgram.get_attr('header', 'daddr')
            dst_addr = ipv4_dgram.get_attr('header', 'saddr')

            # other tcp attributes:
            #   - src & dst ports
            src_port = tcp_seg_in.get_attr('header', 'dst_port')
            dst_port = tcp_seg_in.get_attr('header', 'src_port')
            #   - seq & ack nrs
            ack_nr_out = seq_nr_in + 1
            seq_nr_out = 555
            #   - flags
            flags = (SYN | ACK)

            # prepare outgoing tcp segment
            tcp_seg_out = TCP_Seg(src_addr = src_addr, dst_addr = dst_addr,
                src_port = src_port,
                dst_port = dst_port,
                seq_number = seq_nr_out,
                ack_number = ack_nr_out,
                flags = flags)

            print("tcp_input::process_tcp_seg() [INFO] sending tcp seg : ")
            print(str(tcp_seg_out))
            print('tcp flags: %s' % (str(TCP_Flags(flags_hex = flags))))

            # send out the outgoing tcp segment
            self.stack.ipv4_mod.send_dgram(
                src_ip  = src_addr,
                dst_ip  = dst_addr,
                proto = IPv4_Dgram.IPv4_PROTO_TCP,
                payload = tcp_seg_out.pack())

        elif (self.state == SYN_RECEIVED) and (tcp_flags_in & ACK):
        	self.state = ESTABLISHED

        else:
        	print('something went wrong...')
