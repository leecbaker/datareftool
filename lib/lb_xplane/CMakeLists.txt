FILE(GLOB LB_XPLANE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LB_XPLANE_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(lb_xplane STATIC ${LB_XPLANE_SRC} ${LB_XPLANE_HDR})
target_include_directories(lb_xplane INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(lb_xplane PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

if(UNIX AND NOT APPLE)
    set_property(TARGET lb_xplane PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_xplane_sdk_definitions(lb_xplane 301)

target_link_libraries(lb_xplane PUBLIC lb_filesystem glew)


if(WIN32)
    find_package(ZLIB REQUIRED)
	find_package(libpng CONFIG REQUIRED)
	target_link_libraries(lb_xplane PUBLIC png_static z)
else()
	FIND_LIBRARY(PNG_LIBRARY NAMES libpng16.a libpng.a)
    target_link_libraries(lb_xplane PUBLIC ${PNG_LIBRARY} z)

    if(APPLE)
		FIND_LIBRARY(COCOA_LIBRARY Cocoa)
		target_link_libraries(lb_xplane PUBLIC ${COCOA_LIBRARY})
    endif()
endif(WIN32)