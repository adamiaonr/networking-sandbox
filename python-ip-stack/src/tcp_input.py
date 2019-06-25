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

        # # FIXME : prints for debugging
        # print("tcp_input::process_tcp_seg() [INFO] received tcp seg : ")
        # tcp_seg_in.pseudo_header['src_addr']['value'] = ipv4_dgram.get_attr('header', 'saddr')
        # tcp_seg_in.pseudo_header['dst_addr']['value'] = ipv4_dgram.get_attr('header', 'daddr')
        # print(str(tcp_seg_in))
        # print('tcp flags: %s' % (str(TCP_Flags(flags_hex = tcp_flags_in))))

        # use this as reference for the tcp state machine:
        # https://en.wikipedia.org/wiki/Transmission_Control_Protocol#/media/File:Tcp_state_diagram_fixed_new.svg
        if (self.state == LISTEN) and ((tcp_flags_in & 0x01FF) & SYN):
            # SYN received : 
            #   - update state to SYN_RECEIVED
            self.state = SYN_RECEIVED
            #   - answer back w/ SYN_ACK
            # prepare tcp pseudo header attributes (for cksum calculation)
            src_addr = ipv4_dgram.get_attr('header', 'daddr')
            dst_addr = ipv4_dgram.get_attr('header', 'saddr')
            # other attributes:
            #   - src & dst ports
            src_port = tcp_seg_in.get_attr('header', 'dst_port')
            dst_port = tcp_seg_in.get_attr('header', 'src_port')
            #   - seq & ack nrs
            ack_nr_out = seq_nr_in + 1
            seq_nr_out = 555
            #   - flags
            flags = (SYN | ACK)
            # create outgoing tcp segment
            tcp_seg_out = TCP_Seg(src_addr = src_addr, dst_addr = dst_addr,
                src_port = src_port,
                dst_port = dst_port,
                seq_number = seq_nr_out,
                ack_number = ack_nr_out,
                flags = flags)

            # print("tcp_input::process_tcp_seg() [INFO] sending tcp seg : ")
            # print(str(tcp_seg_out))
            # print('tcp flags: %s' % (str(TCP_Flags(flags_hex = flags))))

            # send outgoing tcp segment
            self.stack.ipv4_mod.send_dgram(
                src_ip  = src_addr,
                dst_ip  = dst_addr,
                proto = IPv4_Dgram.IPv4_PROTO_TCP,
                payload = tcp_seg_out.pack())

        elif (self.state == SYN_RECEIVED) and (tcp_flags_in & ACK):
            # SYN received : 
            #   - update state to ESTABLISHED
            self.state = ESTABLISHED

        elif (tcp_flags_in & (RST)):
            # RST received : unusual event
            #   - update state to LISTEN
            self.state = LISTEN
            
