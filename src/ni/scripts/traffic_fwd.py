#!/usr/bin/python
import sys
from socket import *
bufsize = 8096 # Modify to suit your needs

def forward(data, src_port, src_host, dst_port, dst_host):
    print 'forward from %s:%s to %s:%s' % (src_host, src_port, dst_host, dst_port)
    sock = socket(AF_INET, SOCK_DGRAM)
    sock.sendto(data, (dst_host, int(dst_port)))

def listen(src_host, src_port, dest_host, dest_port):
    listenSocket = socket(AF_INET, SOCK_DGRAM)
    listenSocket.bind((src_host, int(src_port)))
    while True:
        data, addr = listenSocket.recvfrom(bufsize)
        forward(data, addr[1], addr[0], dest_port, dest_host) # data and port

listen(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
