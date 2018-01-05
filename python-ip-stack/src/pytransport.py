import os
import hashlib
import time
import ipaddress
# finally use zeromq (after looked at it in the long gone summer of 2014...)
import zmq
import thread

from udp import UDP_Dgram
from metaframe import MetaFrame
from collections import defaultdict

INADDR_ANY = 0x0000

class PyPacket:

    def __init__(self, 
        peer = None,
        data = ''):

        self.peer = peer
        self.data = data

def listen(transp_mod):

    context = zmq.Context()
    # zmq.REP is for the server side, the pytransport layer listens on 
    # tcp port 5555
    socket = context.socket(zmq.REP)
    socket.bind('tcp://127.0.0.1:5555')
    
    while True:
    
        # block till next message is received
        req = socket.recv().split(';')
    
        if req[0] == 'bind':

            # extract command params
            socket_id = req[1]
            ip = req[2]
            port = int(req[3])

            # convert ip and port params to integer format
            if ip == str(INADDR_ANY):
                ip = transp_mod.stack.ip
            else:
                ip = int(ipaddress.IPv4Address(ip))

            # if port already in use, abort bind
            if port in transp_mod.port_table:
                socket.send('bind;e;port already in use : %d' % (port))
                continue

            # add record to pytransport's port table
            transp_mod.port_table[port] = socket_id

            # send 's'uccess response back to pysocket api
            socket.send('bind;s;%s;%s;%d' % (socket_id, str(ipaddress.IPv4Address(ip)), port))

        elif req[0] == 'send':

            socket_id = req[1]
            ip = int(ipaddress.IPv4Address(req[2]))
            port = int(req[3])
            data = req[4]

            if socket_id not in transp_mode.pysock_table:
                socket.send('send;e;socket w/ id %s unknown' % (socket_id))
                continue

            transp_mod.send(transp_mode.pysock_table[socket_id], PyPacket((ip, port), data))
            socket.send('send;s;%s;%d' % (socket_id, len(data)))

        else:
            print("pytransport::listen() [ERROR] unknown command : %s" % (req[0]))

class PyTransport:

    """manages the transport layer of the python tcp/ip stack"""

    def __init__(self, stack):

        self.stack = stack

        # lookup tables: 
        #   - port_table :  links (ip, port number, proto) tuples to id of pysocket
        self.port_table = defaultdict()
        self.pysock_table = defaultdict()

        # start thread to listen from pysocket API calls 
        try:
            thread.start_new_thread(listen, (self,))
        except:
            print("pytransport::init() [ERROR] error while starting thread...")

    def process_dgram(self, ipv4_dgram):

        # ipv4 datagram should encapsulate either a tcp or udp dgram
        if ipv4_dgram.get_attr('header', 'proto') == IPv4_Dgram.IPv4_PROTO_UDP:

            # unpack ipv4 payload as a udp datagram
            udp_dgram = UDP_Dgram()
            udp_dgram.unpack(ipv4_dgram.get_attr('data', 'data'))

            # send the dgram up, to the appropriate socket
            dst_port = int(udp_dgram.get_attr('header', 'dst_port'))
            if dst_port in self.port_table:

                # extract src address (ip, port) and data
                src_addr = (ipv4_dgram.get_attr('header', 'src_ip'), udp_dgram.get_attr('header', 'src_port')) 
                data = udp_dgram.get_attr('data', 'data')

                # add a PyPacket to the receive queue of the respective socket
                self.port_table[dst_port].rcv_queue.append(PyPacket(src_addr, data))

            else:
                print("pytransport::process_dgram() [ERROR] destination port not in use : (%d)" % (dst_port))

    def send(self, socket, packet):

        if socket.protocol == SOCK_DGRAM:

            # prepare udp datagram for sending
            udp_dgram = UDP_Dgram(
                src_ip          = socket.ip,
                src_port        = socket.port,
                dst_ip          = packet.peer[0],
                dst_port        = packet.peer[1],
                udp_data        = packet.data,
                udp_data_size   = len(packet.data))

            # use ipv4 module to encapsulate udp datagram in ipv4, send it
            self.stack.ipv4_mod.send_dgram(
                src_ip  = socket.ip,
                dst_ip  = packet.peer[0],
                proto = IPv4_Dgram.IPv4_PROTO_UDP,
                payload = udp_dgram.pack())

        else:
            print("pytransport::send() [ERROR] unknown protocol : %02x" % (socket.protocol))
