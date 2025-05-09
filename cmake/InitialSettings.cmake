cmake_minimum_required (VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS "Debug")

set(ANDROID_PLATFORM 26)

# Workaround for MacOS 14 error
set(CMAKE_Swift_COMPILER_FORCED 1 CACHE INTERNAL "")
set(CMAKE_OBJC_COMPILER_FORCED 1 CACHE INTERNAL "")
set(CMAKE_OBJCXX_COMPILER_FORCED 1 CACHE INTERNAL "")

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(CMAKE_SYSTEM_VERSION 26)
    set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})
endif()

SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

SET(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")