# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.


cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        udphelp-lib
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        src/main/cpp/publicbase/threadPool.cpp
        src/main/cpp/publicbase/baseFunc.cpp
        src/main/cpp/publicbase/h264Decode.cpp
        src/main/cpp/publicbase/md5.cpp

        src/main/cpp/cryptopp/e2eeMgr.cpp
        src/main/cpp/cryptopp/rc4.cpp

        src/main/cpp/gl/GraphicBuffer.cpp
        src/main/cpp/gl/DynamicLibrary.cpp
        src/main/cpp/gl/GLbase.cpp
        src/main/cpp/gl/Renderer.cpp
        src/main/cpp/gl/TextureRGB.cpp
        src/main/cpp/gl/TextureYUV.cpp

        src/main/cpp/rocker/rockerMgr.cpp
        src/main/cpp/deskTopDraw.cpp

        src/main/cpp/network/syncSendMgr.cpp
        src/main/cpp/network/udpArqHelp.cpp
        src/main/cpp/network/ikcp.c
        src/main/cpp/network/udphelplib.cpp
        src/main/cpp/network/p2p_udp.cpp

        src/main/cpp/audioPlayer.cpp
        src/main/cpp/Native.cpp
        src/main/cpp/serverAgent/baseInfo.cpp
        src/main/cpp/serverAgent/clientAgent.cpp
        src/main/cpp/serverAgent/proxyClient.cpp
)
include_directories(src/main/cpp/ffmpeg/)
include_directories(src/main/cpp/cryptopp/)
include_directories(src/main/cpp/)


add_library(ffmpeg-lib  SHARED IMPORTED )
add_library(opus  SHARED IMPORTED )
add_library(libc++_static  STATIC IMPORTED )
add_library(libcpufeatures  STATIC IMPORTED )
add_library(cryptopp  STATIC IMPORTED )

if(${ANDROID_ABI} STREQUAL "x86")
set_target_properties(ffmpeg-lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/x86/libffmpeg-lib.so)
set_target_properties(opus PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/x86/libopus.so)
set_target_properties(libc++_static PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/x86/libc++_static.a)
set_target_properties(libcpufeatures PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/x86/libcpufeatures.a)
set_target_properties(cryptopp PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/x86/libcryptopp_static.a)
endif(${ANDROID_ABI} STREQUAL "x86")


if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
set_target_properties(ffmpeg-lib PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/armeabi-v7a/libffmpeg-lib.so)
set_target_properties(opus PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/armeabi-v7a/libopus.so)
endif(${ANDROID_ABI} STREQUAL "armeabi-v7a")


if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    set_target_properties(ffmpeg-lib PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/arm64-v8a/libffmpeg-lib.so)
    set_target_properties(opus PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/arm64-v8a/libopus.so)
    set_target_properties(libc++_static PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/arm64-v8a/libc++_static.a)
    set_target_properties(libcpufeatures PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/arm64-v8a/libcpufeatures.a)
    set_target_properties(cryptopp PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/arm64-v8a/libcryptopp_static.a)
endif(${ANDROID_ABI} STREQUAL "arm64-v8a")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )




# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.



target_link_libraries( # Specifies the target library.
        udphelp-lib
        ffmpeg-lib
        opus
        libc++_static
        libcpufeatures
        cryptopp
        EGL
        GLESv3
        OpenSLES
        android
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )

