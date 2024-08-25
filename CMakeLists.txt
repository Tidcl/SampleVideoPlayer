cmake_minimum_required(VERSION 3.5.0)
project(VideoEditor VERSION 0.1.0 LANGUAGES C CXX)

#fltk
find_package(FLTK REQUIRED PATHS C:/Users/xctan/Downloads/fltk-1.3.9-source/fltk-1.3.9/build)
include_directories(${FLTK_INCLUDE_DIR})
link_directories(C:/Users/xctan/Downloads/fltk-1.3.9-source/fltk-1.3.9/build/lib/Debug)

#ffmpeg
include_directories(C:/Users/xctan/Code/ffmpeg-7.0.1/build-release/include)
link_directories(C:/Users/xctan/Code/ffmpeg-7.0.1/build-release/bin)

#zlib
include_directories(C:/Users/xctan/Code/zlib)
link_directories(C:/Users/xctan/Code/zlib/build/Debug)

#opencv
include_directories(C:/Users/xctan/Code/opencv-4.10.0/build/install/include)
link_directories(C:/Users/xctan/Code/opencv-4.10.0/build/install/x64/vc17/lib)
file(GLOB opencv_libs "C:/Users/xctan/Code/opencv-4.10.0/build/install/x64/vc17/lib/*.lib")
#find_package(OPENCV REQUIRED PATHS C:/Users/xctan/Code/opencv-4.10.0/build/install)

#设置运行目录 可以通过set_target_properties
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY C:/Users/xctan/Code/opencv-4.10.0/build/install/x64/vc17/bin)
#set_target_properties(my_executable PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/custom_bin
#)

file(GLOB PlaySrc "./VideoPlay/*.cpp")
# file(GLOB HDR_FILES "./VideoPlay/*.h")

message("..........................................................................................................................................")
message(${opencv_libs})
message(${PlaySrc})
message("..........................................................................................................................................")



add_executable(VideoEditor main.cpp ${PlaySrc})

#set_target_properties(VideoEditor PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY C:/Users/xctan/Code/opencv-4.10.0/build/install/x64/vc17/bin
#)

target_link_libraries(VideoEditor fltkd fltk_zd fltk_jpegd fltk_pngd fltk_imagesd fltk_formsd fltk_gld )
target_link_libraries(VideoEditor avcodec avdevice avfilter avformat avutil swresample swscale)
target_link_libraries(VideoEditor zlibd zlibstaticd)
target_link_libraries(VideoEditor ${opencv_libs})
