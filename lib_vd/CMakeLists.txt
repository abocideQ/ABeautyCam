cmake_minimum_required(VERSION 3.10.2)

include_directories(
        src/main/cpp/utils
        src/main/cpp/ffmpeg/include
        src/main/cpp/ffmpeg/utils
        src/main/cpp/ffmpeg/codec
        src/main/cpp/ffmpeg/record
        src/main/cpp/opengles/include/glm
        src/main/cpp/opengles/utils
        src/main/cpp/opengles/render
        src/main/cpp/opensles/utils
        src/main/cpp/opensles/render
        src/main/cpp/vd/player
        src/main/cpp/vd/camera
)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libavformat.so)
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libavcodec.so)
add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libavutil.so)
add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libavfilter.so)
add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libswscale.so)
add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI}/libswresample.so)
#link_directories(${CMAKE_HOME_DIRECTORY}/src/main/cpp/ffmpeg/include/lib/${ANDROID_ABI})

set(lib_ffmpeg
        avformat
        avcodec
        avfilter
        avutil
        swscale
        swresample
        )

set(lib_native
        EGL
        GLESv3
        OpenSLES
        log
        android
        )

file(GLOB lib_mine
        src/main/cpp/*.cpp
        src/main/cpp/utils/*.cpp
        src/main/cpp/ffmpeg/*.cpp
        src/main/cpp/ffmpeg/utils/*.cpp
        src/main/cpp/ffmpeg/codec/*.cpp
        src/main/cpp/ffmpeg/record/*.cpp
        src/main/cpp/opengles/*.cpp
        src/main/cpp/opengles/utils/*.cpp
        src/main/cpp/opengles/render/*.cpp
        src/main/cpp/opensles/utils/*.cpp
        src/main/cpp/opensles/render/*.cpp
        src/main/cpp/vd/*.cpp
        src/main/cpp/vd/player/*.cpp
        src/main/cpp/vd/camera/*.cpp
        )

add_library(vd_make
        SHARED
        ${lib_mine}
        )

target_link_libraries(vd_make
        ${lib_ffmpeg}
        ${lib_native}
        )
