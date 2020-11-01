FILE(GLOB LB_XPLANE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB LB_XPLANE_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(lb_xplane_xp10 STATIC ${LB_XPLANE_SRC} ${LB_XPLANE_HDR})
add_library(lb_xplane_xp11 STATIC ${LB_XPLANE_SRC} ${LB_XPLANE_HDR})
target_include_directories(lb_xplane_xp10 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lb_xplane_xp11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(lb_xplane_xp10 PRIVATE "/MT$<$<CONFIG:Debug>:d>")
	TARGET_COMPILE_OPTIONS(lb_xplane_xp11 PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

if(UNIX AND NOT APPLE)
	set_property(TARGET lb_xplane_xp10 PROPERTY POSITION_INDEPENDENT_CODE ON)
	set_property(TARGET lb_xplane_xp11 PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_xplane_sdk_definitions(lb_xplane_xp10 210)
add_xplane_sdk_definitions(lb_xplane_xp11 301)

target_link_libraries(lb_xplane_xp10 PUBLIC lb_filesystem glew)
target_link_libraries(lb_xplane_xp11 PUBLIC lb_filesystem glew)


if(WIN32)
    find_package(ZLIB REQUIRED)
	find_package(libpng CONFIG REQUIRED)
	target_link_libraries(lb_xplane_xp10 PUBLIC png_static z)
	target_link_libraries(lb_xplane_xp11 PUBLIC png_static z)
else()
	FIND_LIBRARY(PNG_LIBRARY NAMES libpng16.a libpng.a)
    target_link_libraries(lb_xplane_xp10 PUBLIC ${PNG_LIBRARY} z)
    target_link_libraries(lb_xplane_xp11 PUBLIC ${PNG_LIBRARY} z)

    if(APPLE)
		FIND_LIBRARY(COCOA_LIBRARY Cocoa)
		target_link_libraries(lb_xplane_xp10 PUBLIC ${COCOA_LIBRARY})
		target_link_libraries(lb_xplane_xp11 PUBLIC ${COCOA_LIBRARY})
    endif()
endif(WIN32)