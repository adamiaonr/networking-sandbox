from pysocket import PySocket

if __name__ == "__main__":

    # simple application whichs receives udp packets
    pysckt = PySocket(PySocket.SOCK_DGRAM)
    pysckt.bind(PySocket.INADDR_ANY, 49999)

    print("exiting...")