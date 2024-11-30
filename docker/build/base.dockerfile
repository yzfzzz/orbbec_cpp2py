# ָ����������, �ڸþ����Ϲ�������
FROM  ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

SHELL ["/bin/bash", "-c"]

RUN apt-get clean && \
    apt-get autoclean
COPY apt/sources.list /etc/apt/

RUN apt-get update  && apt-get upgrade -y  && \
    apt-get install -y \
    htop \
    apt-utils \
    curl \
    git \
    openssh-server \
    net-tools \
    vim \
    zip \
    gdb \
    libc-ares-dev \ 
    libssl-dev \
    gcc \
    g++ \
    make 

RUN apt-get install -y libc-ares-dev  libssl-dev gcc g++ make 

RUN apt-get install -y python3-dev \ 
    python3-venv \
    python3-pip \ 
    python3-opencv

COPY cmake /tmp/cmake
RUN ./tmp/cmake/install_cmake.sh

RUN apt-get install -y build-essential \ 
    libgtk2.0-dev \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libjpeg-dev \
    libswscale-dev \
    libtiff5-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer1.0-dev \
    libpng-dev \
    libopenexr-dev \
    libtiff-dev \
    libwebp-dev

COPY opencv /tmp/opencv
RUN ./tmp/opencv/install_opencv.sh






