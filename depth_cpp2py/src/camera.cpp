#include <iostream>
#include <libobsensor/hpp/Context.hpp>
#include <libobsensor/hpp/Device.hpp>
#include <libobsensor/hpp/Error.hpp>
#include <libobsensor/hpp/StreamProfile.hpp>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "camera.h"
// profile：配置
// 编译器会在编译阶段就知晓它的值为 false
constexpr bool FLIP = false;

void Camera::init_color() {
    // 获取彩色相机所有流的配置，包括流的的分辨率，帧率，以及帧的格式
    auto profiles = this->pipe->getStreamProfileList(OB_SENSOR_COLOR);
    std::shared_ptr<ob::VideoStreamProfile> color_profile = nullptr;

    try {
        // 根据指定的的格式查找对应的profile, 优先选择RGB888格式
        color_profile =
            profiles->getVideoStreamProfile(640, 0, OB_FORMAT_RGB888, 30);
    } catch (const ob::Error &) {
        // 没找到RGB888格式后不匹配格式查找对应的Profile进行开流
        color_profile =
            profiles->getVideoStreamProfile(640, 0, OB_FORMAT_UNKNOWN, 30);
    }

    // 开启/关闭相机的镜像模式
    if (this->device->isPropertySupported(OB_PROP_COLOR_MIRROR_BOOL,
                                          OB_PERMISSION_WRITE)) {
        this->device->setBoolProperty(OB_PROP_COLOR_MIRROR_BOOL, FLIP);
    }

    // 开启彩色流
    this->config->enableStream(color_profile);
}

void Camera::init_depth() {
    auto profiles = this->pipe->getStreamProfileList(OB_SENSOR_DEPTH);
    std::shared_ptr<ob::VideoStreamProfile> depth_profile = nullptr;
    try {
        // 根据指定的格式查找对应的Profile,优先查找Y16格式
        depth_profile =
            profiles->getVideoStreamProfile(640, 0, OB_FORMAT_Y16, 30);

    } catch (const ob::Error &) {
        // 没找到Y16格式后不匹配格式查找对应的Profile进行开流
        depth_profile =
            profiles->getVideoStreamProfile(640, 0, OB_FORMAT_UNKNOWN, 30);
    }

    // 开启/关闭深度相机的镜像模式
    if (this->device->isPropertySupported(OB_PROP_DEPTH_MIRROR_BOOL,
                                          OB_PERMISSION_WRITE)) {
        this->device->setBoolProperty(OB_PROP_DEPTH_MIRROR_BOOL, FLIP);
    }

    // 开启深度流
    this->config->enableStream(depth_profile);
}

void Camera::init_IR() {
    // 获取红外相机的所有流配置，包括流的分辨率，帧率，以及帧的格式
    auto profiles = pipe->getStreamProfileList(OB_SENSOR_IR);
    std::shared_ptr<ob::VideoStreamProfile> ir_profile = nullptr;
    try {
        // 根据指定的格式查找对应的Profile,优先查找Y16格式
        ir_profile = profiles->getVideoStreamProfile(640, 0, OB_FORMAT_Y16, 30);
    } catch (const ob::Error &) {
        // 没找到Y16格式后不匹配格式查找对应的Profile进行开流
        ir_profile =
            profiles->getVideoStreamProfile(640, 0, OB_FORMAT_UNKNOWN, 30);
    }
    // 开启/关闭红外相机的镜像模式
    if (this->device->isPropertySupported(OB_PROP_IR_MIRROR_BOOL,
                                          OB_PERMISSION_WRITE)) {
        this->device->setBoolProperty(OB_PROP_IR_MIRROR_BOOL, FLIP);
    }

    // 开启红外流
    this->config->enableStream(ir_profile);
}

Camera::Camera(bool color, bool IR, bool depth)
    : pipe(std::make_shared<ob::Pipeline>()),
      config(std::make_shared<ob::Config>()) {
    this->device = pipe->getDevice();
    if (color) {
        this->init_color();
    }
    if (IR) {
        this->init_IR();
    }
    if (depth) {
        this->init_depth();
    }

    // 若摄像头支持多种流，则开启流同步
    if (color + IR + depth > 1) {
        if (device->isPropertySupported(OB_PROP_DEPTH_ALIGN_HARDWARE_BOOL,
                                        OB_PERMISSION_READ)) {
            config->setAlignMode(ALIGN_D2C_HW_MODE);
        } else {
            config->setAlignMode(ALIGN_D2C_SW_MODE);
        }
    }

    ob::Context ctx;
    ctx.setDeviceChangedCallback(
        [this](std::shared_ptr<ob::DeviceList> removed_devices,
               std::shared_ptr<ob::DeviceList> added_devices) {
            if (added_devices->deviceCount() > 0) {
                this->pipe = std::make_shared<ob::Pipeline>();
                this->start();
            } else if (removed_devices->deviceCount() > 0) {
                this->stop();
            }
        });
}

// 奥比中光frame => cv::Mat
cv::Mat Camera::frame2mat(const std::shared_ptr<ob::VideoFrame> &frame) {
    if (frame == nullptr || frame->dataSize() < 1024) {
        return {};
    }

    const OBFrameType frame_type = frame->type();  // 帧类型（彩色/深度/IR）
    const OBFormat frame_format = frame->format();               // 图像格式
    const int frame_height = static_cast<int>(frame->height());  // 图像高度
    const int frame_width = static_cast<int>(frame->width());    // 图像宽度
    void *const frame_data = frame->data();  // 帧原始数据首地址
    const int data_size = static_cast<int>(frame->dataSize());  // 帧数据大小

    cv::Mat result_mat;

    if (frame_type == OB_FRAME_COLOR) {
        // Color image
        if (frame_format == OB_FORMAT_MJPG) {
            const cv::Mat raw_mat(1, data_size, CV_8UC1, frame_data);
            result_mat = cv::imdecode(raw_mat, 1);
        } else if (frame_format == OB_FORMAT_NV21) {
            const cv::Mat raw_mat(frame_height * 3 / 2, frame_width, CV_8UC1,
                                  frame_data);
            cv::cvtColor(raw_mat, result_mat, cv::COLOR_YUV2BGR_NV21);
        } else if (frame_format == OB_FORMAT_YUYV ||
                   frame_format == OB_FORMAT_YUY2) {
            const cv::Mat raw_mat(frame_height, frame_width, CV_8UC2,
                                  frame_data);
            cv::cvtColor(raw_mat, result_mat, cv::COLOR_YUV2BGR_YUY2);
        } else if (frame_format == OB_FORMAT_RGB888) {
            const cv::Mat raw_mat(frame_height, frame_width, CV_8UC3,
                                  frame_data);
            cv::cvtColor(raw_mat, result_mat, cv::COLOR_RGB2BGR);
        } else if (frame_format == OB_FORMAT_UYVY) {
            const cv::Mat raw_mat(frame_height, frame_width, CV_8UC2,
                                  frame_data);
            cv::cvtColor(raw_mat, result_mat, cv::COLOR_YUV2BGR_UYVY);
        }
    } else if (frame_format == OB_FORMAT_Y16 ||
               frame_format == OB_FORMAT_YUYV ||
               frame_format == OB_FORMAT_YUY2) {
        // IR or depth image
        cv::Mat raw_mat(frame_height, frame_width, CV_16UC1, frame_data);
        // cv::imwrite("cv_raw_depth.png", raw_mat);
        const double scale =
            1 / pow(2, frame->pixelAvailableBitSize() -
                           (frame_type == OB_FRAME_DEPTH ? 10 : 8));
        // cv::convertScaleAbs(raw_mat, result_mat, scale);
        return raw_mat;
        
    } else if (frame_type == OB_FRAME_IR) {
        // IR image
        if (frame_format == OB_FORMAT_Y8) {
            result_mat =
                cv::Mat(frame_height, frame_width, CV_8UC1, frame_data);
        } else if (frame_format == OB_FORMAT_MJPG) {
            const cv::Mat raw_mat(1, data_size, CV_8UC1, frame_data);
            result_mat = cv::imdecode(raw_mat, 1);
        }
    }
    return result_mat;
}
