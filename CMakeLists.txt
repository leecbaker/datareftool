project(datareftool C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

# X-Plane 11's minimum OS version is 10.10
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

IF( NOT CMAKE_BUILD_TYPE )
	SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()
message(STATUS "Current build type is : ${CMAKE_BUILD_TYPE}")

add_subdirectory(lib)
add_subdirectory(src)
cmake_minimum_required(VERSION 2.8)
