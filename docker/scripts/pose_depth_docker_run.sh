#!/usr/bin/env bash
MONITOR_HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." && pwd )"

display=""
if [ -z ${DISPLAY} ];then
    display=":1"
else
    display="${DISPLAY}"
fi

local_host="$(hostname)"
user="${USER}"
uid="$(id -u)"
group="$(id -g -n)"
gid="$(id -g)"


echo "stop and rm docker" 
docker stop pose_depth_ob > /dev/null
docker rm -v -f pose_depth_ob > /dev/null

echo "start docker"
docker run -it -d \
--privileged \
--name pose_depth_ob \
-e DISPLAY=$display \
--privileged=true \
-e DOCKER_USER="${user}" \
-e USER="${user}" \
-e DOCKER_USER_ID="${uid}" \
-e DOCKER_GRP="${group}" \
-e DOCKER_GRP_ID="${gid}" \
-e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
-v ${MONITOR_HOME_DIR}:/home/work \
-v ${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR} \
-v /dev/bus/usb:/dev/bus/usb \
--net host \
opencv:4.5
