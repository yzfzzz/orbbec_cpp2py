#pragma once

#include <initializer_list>
#include <libobsensor/hpp/Frame.hpp>
#include <libobsensor/hpp/Pipeline.hpp>
#include <libobsensor/hpp/Types.hpp>
#include <opencv2/core.hpp>

class Camera {
   private:
    std::shared_ptr<ob::Device> device;
    std::shared_ptr<ob::Config> config;
    std::shared_ptr<ob::Pipeline> pipe;

    // 设置彩色图/深度图/红外图的相机参数，如长宽/图片格式等
    void init_color();
    void init_IR();
    void init_depth();

   public:
    Camera(bool color, bool IR, bool depth);
    inline ~Camera() { this->pipe->stop(); }
    inline void start() { this->pipe->start(this->config); }
    inline void stop() { this->pipe->stop(); }
    inline std::shared_ptr<ob::FrameSet> get() {
        return pipe ? pipe->waitForFrameset(100) : nullptr;
    }
    static cv::Mat frame2mat (const std::shared_ptr<ob::VideoFrame> &frame);
};