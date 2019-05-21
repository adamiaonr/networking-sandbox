import time
import ipaddress
import hashlib
import zmq

from pytransport import PyPacket
from collections import defaultdict

# def listen(transp_mod):

#     context = zmq.Context()

#     # zmq.REP is for the server side, the pysocket_module listens on 
#     # tcp port 5556
#     socket = context.socket(zmq.REP)
#     socket.bind('tcp://127.0.0.1:5556')
    
#     while True:
    
#         # block till next message is received
#         msg = socket.recv().split('|')
    
#         if msg[0] == 'RECV':

#             # socket assoc. w/ the command
#             socket_id = msg[1]



#         else:

#             print("pytransport::listen() [ERROR] unknown message code : %s" % (msg[0]))

# class PySocket_Module:

#     def __init__(self, stack):

#         # lookup tables: 
#         #   - socket_table :  links socket_id to pysocket objects
#         self.socket_table = defaultdict()

#         # start thread to listen from calls from pytransport
#         try:
#             thread.start_new_thread(listen, (self,))
#         except:
#             print("pysocket_module::init() [ERROR] error while starting thread...")

class PySocket:

    """a simplified C-like socket api for apps to use my python tcp/ip stack"""

    # bind to Stack's ip address (note this has a different meaning from the C API, 
    # in which INADDR_ANY is used to bind the socket to all local interfaces)
    INADDR_ANY = 0x0000

    # only 2 types of sockets:
    #   -# SOCK_DGRAM   : udp socket
    #   -# SOCK_STREAM  : tcp socket
    SOCK_DGRAM  = 0x00
    SOCK_STREAM = 0x01

    # send and receiver buffer limits (# of packets received, but not 
    # delivered to application)
    MAX_RECV_WIN_SIZE = 256

    def __init__(self, 
        protocol = SOCK_DGRAM):

        self.protocol = protocol
        self.id = hashlib.md5(str(self.protocol) + str(time.time())).hexdigest()

        self.ip = None
        self.port = None

        # send and receive queues (# Packet objects not bytes)
        # FIXME: this will come back to haunt me w/ tcp
        self.snd_queue = []
        self.rcv_queue = []

    def bind(self, ip, port):

        # connect to pytransport
        context = zmq.Context()
        socket = context.socket(zmq.REQ)
        socket.connect('tcp://127.0.0.1:5555')

        # send BIND request to pytransport            
        socket.send('bind;%s;%d;%s;%d' % (self.id, self.protocol, str(ip), port))
        # wait for reply
        reply = socket.recv().split(';')
        # 's' means success, 'e' means error
        if reply[1] == 's':
            
            # success : update final (ip address, port)
            self.ip = int(ipaddress.IPv4Address(unicode(reply[3])))
            self.port = int(reply[4])

            print("pysocket::bind() [INFO] socket %s bound to (%s, %d)" % (reply[2], reply[3], int(reply[4])))

            return 0

        else:
            print("pysocket::bind() [ERROR] can't bind to (%s, %d) : %s" % (str(ip), port, reply[2]))

        return -1

    def recv(self, data):

        # FIXME: block until rcv queue has something on it
        while(len(self.rcv_queue) == 0):
            time.sleep(1)

        # keep concatenating data of packets in the queue, assume src address 
        # is the same for all packets in the queue (this may not be true for udp packets)
        # FIXME: this needs a lot of improvements, but for a basic recv 
        # routine, this is enough
        for packet in self.recv_queue:
            data += str(packet.data)

        src_addr = self.rcv_queue[0].peer
        src_addr = (str(ipaddress.IPv4Address(src_addr[0])), int(src_addr[1]))

        # clear the recv queue
        self.rcv_queue = []

        return len(data), src_addr

    def send(self, data, dst_addr = None):

        # connect to pytransport
        context = zmq.Context()

        socket = context.socket(zmq.REQ)
        socket.connect('tcp://127.0.0.1:5555')

        socket.send('send;%s;%s;%d;%s' % (self.id, dst_addr[0], dst_addr[1], data))
        reply = socket.recv().split(';')

        if reply[1] == 's':

            print("pysocket::send() [INFO] sent %d bytes over socket id %s to (%s, %d)" % (int(reply[3], reply[2], dst_addr[0], dst_addr[1])))
            return 0

        else:
            print("pysocket::send() [ERROR] couldn't send data : %s" % (reply[2]))

        return -1