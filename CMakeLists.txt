cmake_policy(VERSION 3.13)

project(datareftool LANGUAGES C CXX VERSION 2.0.0)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -ftemplate-backtrace-limit=0 -ferror-limit=100 -Wold-style-cast")
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -static-libstdc++")
endif()

add_subdirectory(lib)
add_subdirectory(src)

