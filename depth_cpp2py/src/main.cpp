#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <time.h>
#include <mutex>
#include <thread>
#include "utils.hpp"
#include "libobsensor/ObSensor.hpp"
#include "libobsensor/hpp/Error.hpp"

int main(int argc, char **argv)
{
    // 定义socket信息
    char *servInetAddr = "127.0.0.1";
    int servPort = 6666;
    int connfd;
    struct sockaddr_in addr;
    // 创建socket
    connfd = socket(AF_INET,SOCK_STREAM, 0);
    if (connfd == -1)
    {
        std::cout<<"socket创建失败"<<std::endl;
        exit(-1);
    }
 
    // 准备通信地址
    addr.sin_family=AF_INET;
    addr.sin_port=htons(servPort);
    addr.sin_addr.s_addr = inet_addr(servInetAddr);
 
    // bind
    int res = connect(connfd,(struct sockaddr*)&addr,sizeof(addr));
    if(res==-1)
    {
        std::cout<<"bind连接失败"<<std::endl;
        exit(-1);
    }
    std::cout<<"bind连接成功"<<std::endl;
 
    // 获取视频帧并发送
    // Create a pipeline with default device.
    ob::Pipeline pipe;
    // Configure which streams to enable or disable for the Pipeline by creating a Config.
    std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
    // Enable color video stream.
    config->enableVideoStream(OB_STREAM_COLOR);
    // Start the pipeline with config.
    pipe.start(config);
    cv::Mat img;
    std::vector<uchar> data_encode;
    
    while(true){

        auto frameSet = pipe.waitForFrameset();
        if(frameSet == nullptr) {
            continue;
        }
        // get color frame from frameset.
        auto colorFrame = frameSet->getFrame(OB_FRAME_COLOR);

        void *const frame_data = colorFrame->data();                     // 帧原始数据首地址
        const int data_size = static_cast<int>(colorFrame->dataSize());  // 帧数据大小
        cv::Mat raw_mat(1, data_size, CV_8UC1, frame_data);
        img = cv::imdecode(raw_mat, 1);
 
        cv::imencode(".jpg",img,data_encode);
        int len_encode = data_encode.size();
        std::string len = std::to_string(len_encode);
        int length = len.length(); 
        for (int i=0;i<16-length;i++) len=len+' ';
        //cout<<len.c_str()<<' '<<strlen(len.c_str())<<endl;
        clock_t start = clock();
 
        // 发送数据
        send(connfd,len.c_str(),strlen(len.c_str()),0);
 
   	    std::string str_encode(data_encode.begin(), data_encode.end());
	    send(connfd,str_encode.data(),len_encode,0);    
        clock_t end   = clock();
        std::cout<<end - start<<'@'<<CLOCKS_PER_SEC<<std::endl;   
        // 接收返回信息
        char recvBuf[32] = "";
        if(recv(connfd, recvBuf,32,0)) std::cout<<recvBuf<<std::endl;
    }
    pipe.stop();
 
    close(connfd);
    return 0;
 
}