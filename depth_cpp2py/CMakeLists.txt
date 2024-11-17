# cmake needs this line
cmake_minimum_required(VERSION 3.10)
# set(CMAKE_BUILD_TYPE "Debug")
# Define project name
project(depth_cpp2py)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(OrbbecSDK_DIR lib/orbbecsdk/lib)
find_package(OrbbecSDK REQUIRED)
find_package(OpenCV REQUIRED)
 
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

add_executable(send src/main.cpp src/camera.cpp)

target_include_directories(send PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/orbbecsdk/examples/src/utils
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(send ${OpenCV_LIBS} ob::OrbbecSDK)