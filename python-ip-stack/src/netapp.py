from pysocket import PySocket

if __name__ == "__main__":

    # simple application whichs receives udp packets
    pysckt = PySocket(PySocket.SOCK_DGRAM)
    pysckt.bind(PySocket.INADDR_ANY, 49999)
    pysckt.send('login', ('192.168.131.172', 31337))

    print("exiting...")