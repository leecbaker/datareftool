SET(SRC clipboard.h)

if(APPLE)
	list(APPEND SRC "clipboard_mac.mm")
elseif(WIN32)
	list(APPEND SRC "clipboard_win.cpp")
else()
	list(APPEND SRC "clipboard_linux.cpp")
endif()

add_library(clipboard STATIC ${SRC})

target_include_directories(clipboard PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    set_property(TARGET clipboard PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
