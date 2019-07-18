if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

IF( NOT CMAKE_BUILD_TYPE )
	SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()
message(STATUS "Current build type is : ${CMAKE_BUILD_TYPE}")

add_subdirectory(lib)
add_subdirectory(src)
cmake_minimum_required(VERSION 2.8)
