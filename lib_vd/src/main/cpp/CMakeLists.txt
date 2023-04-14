cmake_minimum_required(VERSION 3.10.2)

#=====ffmpeg.so
add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libavformat.so)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libavcodec.so)
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libavutil.so)
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libavfilter.so)
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libswscale.so)
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ffmpeg/include/lib/${ANDROID_ABI}/libswresample.so)
#link_directories(${CMAKE_HOME_DIRECTORY}/ffmpeg/include/lib/${ANDROID_ABI})
set(lib_ffmpeg
        avformat
        avcodec
        avfilter
        avutil
        swscale
        swresample
        )

#=====opencv.so
add_library(opencv_java4 SHARED IMPORTED)
set_target_properties(opencv_java4 PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/opencv/include/lib/${ANDROID_ABI}/libopencv_java4.so)
set(lib_opencv
        opencv_java4
        )

#=====headers
# ffmpeg + opengl + opensl + opencv
# alignment(faceAlignment 5 point of face landmark)
# facecnn(libfacedetection)
# vd
include_directories(
        ${CMAKE_SOURCE_DIR}

        ${CMAKE_SOURCE_DIR}/utils

        ${CMAKE_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/ffmpeg/include
        ${CMAKE_SOURCE_DIR}/ffmpeg/utils
        ${CMAKE_SOURCE_DIR}/ffmpeg/codec
        ${CMAKE_SOURCE_DIR}/ffmpeg/record

        ${CMAKE_SOURCE_DIR}/opengles/include/glm
        ${CMAKE_SOURCE_DIR}/opengles/utils
        ${CMAKE_SOURCE_DIR}/opengles/render

        ${CMAKE_SOURCE_DIR}/opensles/utils
        ${CMAKE_SOURCE_DIR}/opensles/render

        ${CMAKE_SOURCE_DIR}/opencv/include
        ${CMAKE_SOURCE_DIR}/opencv/render

        ${CMAKE_SOURCE_DIR}/alignment/include/alignment

        ${CMAKE_SOURCE_DIR}/facecnn/include
        ${CMAKE_SOURCE_DIR}/facecnn/render

        ${CMAKE_SOURCE_DIR}/vd/player
        ${CMAKE_SOURCE_DIR}/vd/camera
        ${CMAKE_SOURCE_DIR}/vd/record
)

#======cpps
# ffmpeg + opengl + opensl + opencv
# alignment(faceAlignment 5 point of face landmark)
# facecnn(libfacedetection)
# vd
file(GLOB lib_mine
        ${CMAKE_SOURCE_DIR}/*.cpp
        ${CMAKE_SOURCE_DIR}/utils/*.cpp

        ${CMAKE_SOURCE_DIR}/src/*.cpp

        ${CMAKE_SOURCE_DIR}/ffmpeg/*.cpp
        ${CMAKE_SOURCE_DIR}/ffmpeg/utils/*.cpp
        ${CMAKE_SOURCE_DIR}/ffmpeg/codec/*.cpp
        ${CMAKE_SOURCE_DIR}/ffmpeg/record/*.cpp

        ${CMAKE_SOURCE_DIR}/opengles/*.cpp
        ${CMAKE_SOURCE_DIR}/opengles/utils/*.cpp
        ${CMAKE_SOURCE_DIR}/opengles/render/*.cpp

        ${CMAKE_SOURCE_DIR}/opensles/utils/*.cpp
        ${CMAKE_SOURCE_DIR}/opensles/render/*.cpp

        ${CMAKE_SOURCE_DIR}/opencv/*.cpp
        ${CMAKE_SOURCE_DIR}/opencv/render/*.cpp

        ${CMAKE_SOURCE_DIR}/alignment/include/alignment/*.cpp

        ${CMAKE_SOURCE_DIR}/facecnn/*.cpp
        ${CMAKE_SOURCE_DIR}/facecnn/include/*.cpp
        ${CMAKE_SOURCE_DIR}/facecnn/render/*.cpp

        ${CMAKE_SOURCE_DIR}/vd/*.cpp
        ${CMAKE_SOURCE_DIR}/vd/player/*.cpp
        ${CMAKE_SOURCE_DIR}/vd/camera/*.cpp
        ${CMAKE_SOURCE_DIR}/vd/record/*.cpp
        )

#======native libs opengles etc.
set(lib_native
        EGL
        GLESv3
        OpenSLES
        log
        android
        )

add_library(vd_make
        SHARED
        ${lib_mine}
        )

target_link_libraries(vd_make
        ${lib_ffmpeg}
        ${lib_opencv}
        ${lib_ncnn}
        ${lib_native}
        )
