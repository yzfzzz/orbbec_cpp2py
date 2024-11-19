#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <libobsensor/hpp/Error.hpp>
#include <memory>
#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>
#include "utils.hpp"

#include "camera.h"

cv::Point clickedPoint;
bool isClicked = false;

void mouseCallback(int event, int x, int y, int flags, void *userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        clickedPoint.x = x;
        clickedPoint.y = y;
        isClicked = true;
    }
}

void send_img(cv::Mat img, int &connfd) {
    std::vector<uchar> data_encode;
    cv::imencode(".png", img, data_encode);
    // std::cout << data_encode.size() << std::endl;
    int len_encode = data_encode.size();
    std::string len = std::to_string(len_encode);
    int length = len.length();
    for (int i = 0; i < 16 - length; i++) len = len + ' ';
    // cout<<len.c_str()<<' '<<strlen(len.c_str())<<endl;
    clock_t start = clock();

    // 发送数据
    send(connfd, len.c_str(), strlen(len.c_str()), 0);

    std::string str_encode(data_encode.begin(), data_encode.end());
    send(connfd, str_encode.data(), len_encode, 0);
    clock_t end = clock();
    // std::cout << end - start << '@' << CLOCKS_PER_SEC << std::endl;
    // 接收返回信息
    char recvBuf[32] = "";
    // if (recv(connfd, recvBuf, 32, 0)) std::cout << recvBuf << std::endl;
}

int main(void) {
    try {
        Camera camera(true, true, true);    
        camera.start();

        // 定义socket信息
        char *servInetAddr = "127.0.0.1";
        int servPort = 6666;
        int connfd;
        struct sockaddr_in addr;
        // 创建socket
        connfd = socket(AF_INET, SOCK_STREAM, 0);
        if (connfd == -1) {
            std::cout << "socket创建失败" << std::endl;
            exit(-1);
        }

        // 准备通信地址
        addr.sin_family = AF_INET;
        addr.sin_port = htons(servPort);
        addr.sin_addr.s_addr = inet_addr(servInetAddr);

        // bind
        int res = connect(connfd, (struct sockaddr *)&addr, sizeof(addr));
        if (res == -1) {
            std::cout << "bind连接失败" << std::endl;
            exit(-1);
        }
        std::cout << "bind连接成功" << std::endl;

        cv::namedWindow("cpp-Color");
        cv::setMouseCallback("cpp-Color", mouseCallback, nullptr);
        cv::Mat depth_img;
        cv::Mat img;
        cv::Mat ir_img;
        while (cv::waitKey(1) != 27) {  // ESC的ASCII码是27，按ESC键可以终止程序
            auto frame_set = camera.get();
            if (frame_set != nullptr) {
                img = camera.frame2mat(frame_set->colorFrame());
                depth_img = camera.frame2mat(frame_set->depthFrame());
                if (!img.empty() && !depth_img.empty()) {
                    // -------------网络传输图片------------
                    send_img(img, connfd);
                    send_img(depth_img, connfd);
                    cv::imshow("cpp-Color", img);
                    cv::imshow("cpp-Depth", depth_img);
                    if (isClicked) {
                        std::cout << "鼠标点击坐标: (" << clickedPoint.x << ", "
                                  << clickedPoint.y << ")"
                                  << " 距离: "
                                  //   at.(i,j)指像素点的位置，表示第i行第j列。
                                  << 64*depth_img.at<u_int8_t>(clickedPoint.y,
                                                          clickedPoint.x)
                                  << std::endl;
                        isClicked = false;
                    }
                }
            }
        }
        close(connfd);
        camera.stop();
        cv::destroyAllWindows();
    } catch (const ob::Error &e) {
        std::cerr << "Function:" << e.getName() << "\nargs:" << e.getArgs()
                  << "\nmessage:" << e.getMessage()
                  << "\ntype:" << e.getExceptionType() << std::endl;
        exit(EXIT_FAILURE);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Unexpected Error!" << std::endl;
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}