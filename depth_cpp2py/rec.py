#-- coding:UTF-8 --
#!/usr/bin/python3
import socket
import os
import sys
import struct
import numpy as np
import cv2
import time
import json


# --------------------------鼠标回调函数---------------------------------------------------------
#   event               鼠标事件
#   param               输入参数
# -----------------------------------------------------------------------------------------------
def onmouse_pick_points(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        print('\npixel: x = %d, y = %d' % (x, y))
        # print("世界坐标是：", threeD[y][x][0], threeD[y][x][1], threeD[y][x][2], "mm")
        distance = param[y][x]*64
        print("distance:", distance, "mm")

# mode: 0灰度图 1彩色图
def recv_img(sock,image_name, mode):
    fileinfo_size = struct.calcsize('16s')
    buf = sock.recv(16)
    filesize = int(struct.unpack('16s', buf)[0].decode())
    # print(filesize)
    b1 = bytearray()
    recvd_size=0  
    t0 =time.time()
    while not len(b1) == filesize: 
        if filesize - recvd_size > 1024:
            data = sock.recv(1024)
            recvd_size += len(data)
        else:
            data = sock.recv(filesize - recvd_size)
        b1.extend(data)
    # print(len(b1))
    image = np.asarray(b1, dtype="uint8") # shape: (481755,)

    # 编码后的数据自带图像的宽/高
    image = cv2.imdecode(image, mode)
    cv2.setMouseCallback(image_name, onmouse_pick_points, image)
    cv2.imshow(image_name,image)
    # print('Image recieved successfully!fps:'+str(1/(time.time()-t0)))
    sock.send('recieved messages!'.encode())     


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
        recv_img(sock,"python-Color", 1)
        recv_img(sock,"python-Depth", 0)
        cv2.waitKey(1)

        
   
if __name__ == '__main__':
    socket_service_image()