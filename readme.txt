python装av库之前需要装这个包：
装pkg_config: https://blog.csdn.net/Charliewolf/article/details/101273248

安装ffmpeg
apt-get install -y autoconf automake build-essential git libass-dev libfreetype6-dev libsdl2-dev libtheora-dev libtool libva-dev libvdpau-dev libvorbis-dev libxcb1-dev libxcb-shm0-dev libxcb-xfixes0-dev pkg-config texinfo wget zlib1g-dev

apt-get install -y libavformat-dev libavcodec-dev libavdevice-dev libavutil-dev libswscale-dev libavresample-dev

apt install libswresample-dev
apt install libswscale-dev
apt install libavutil-dev
apt install libsdl1.2-dev

再真正安装ffmpeg
https://blog.csdn.net/weixin_42329133/article/details/122329663

export PKG_CONFIG_PATH="/usr/lib/pkgconfig"


