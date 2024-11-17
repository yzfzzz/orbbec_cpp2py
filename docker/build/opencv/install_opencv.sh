#!/usr/bin/env bash

set -e

cd "$(dirname "${BASH_SOURCE[0]}")"

THREAD_NUM=$(nproc)

# https://github.com/Kitware/CMake/archive/refs/tags/v3.26.4.tar.gz
# Install cmake.
VERSION="4.5.0"
PKG_NAME="opencv-${VERSION}.zip"

unzip "${PKG_NAME}"
pushd opencv-${VERSION}
    mkdir build
    pushd build
    cmake -D CMAKE_BUILD_TYPE=Release -D OPENCV_GENERATE_PKGCONFIG=YES ..
    make
    make install
    echo "export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH" >> /etc/profile.d/pkgconfig.sh
    source /etc/profile
    echo "/usr/local/lib" >> /etc/ld.so.conf.d/opencv4.conf
    ldconfig
    popd
popd


rm -rf PKG_NAME opencv-${VERSION}

