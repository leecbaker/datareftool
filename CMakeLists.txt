cmake_policy(VERSION 3.15)

project(datareftool LANGUAGES C CXX VERSION 2.0.0)

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -ftemplate-backtrace-limit=0 -ferror-limit=100")
endif()

if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

add_subdirectory(lib)
add_subdirectory(src)

