#-- coding:UTF-8 --
#!/usr/bin/python3
import socket
import os
import sys
import struct
import numpy as np
import cv2
import time

def socket_service_image():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('', 6666))
        s.listen(True)
    except socket.error as msg:
        print(msg)
        sys.exit(1)
 
    print("Wait for Connection.....................")
    sock, addr = s.accept()  
    while True:
        fileinfo_size = struct.calcsize('16s')
        buf = sock.recv(16)
        filesize = int(struct.unpack('16s', buf)[0].decode())
        print(filesize)
        b1 = bytearray()
        recvd_size=0 
        t0 =time.time()
        while not len(b1) == filesize: #必须保证完全接收
            if filesize - recvd_size > 1024:
                data = sock.recv(1024)
                recvd_size += len(data)
            else:
                data = sock.recv(filesize - recvd_size)
            b1.extend(data)
        print(len(b1))
        image = np.asarray(b1, dtype="uint8")
        image = cv2.imdecode(image, cv2.IMREAD_COLOR)
        cv2.imshow('',image)
        cv2.waitKey(1)
        print('Image recieved successfully!fps:'+str(1/(time.time()-t0)))
        sock.send('recieved messages!'.encode())      
   
if __name__ == '__main__':
    socket_service_image()