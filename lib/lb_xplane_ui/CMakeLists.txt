FILE(GLOB LB_XPLANE_UI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/container/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.cpp")
FILE(GLOB LB_XPLANE_UI_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/container/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.h")

add_library(lb_xplane_ui_xp10 STATIC ${LB_XPLANE_UI_SRC} ${LB_XPLANE_UI_HDR})
add_library(lb_xplane_ui_xp11 STATIC ${LB_XPLANE_UI_SRC} ${LB_XPLANE_UI_HDR})
target_include_directories(lb_xplane_ui_xp10 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lb_xplane_ui_xp11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    TARGET_COMPILE_OPTIONS(lb_xplane_ui_xp10 PRIVATE "/MT$<$<CONFIG:Debug>:d>")
    TARGET_COMPILE_OPTIONS(lb_xplane_ui_xp11 PRIVATE "/MT$<$<CONFIG:Debug>:d>")

    target_link_libraries(lb_xplane_ui_xp10 INTERFACE Opengl32.lib)
    target_link_libraries(lb_xplane_ui_xp11 INTERFACE Opengl32.lib)
endif(WIN32)

if(UNIX AND NOT APPLE)
    set_property(TARGET lb_xplane_ui_xp10 PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_property(TARGET lb_xplane_ui_xp11 PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(lb_xplane_ui_xp10 PUBLIC clipboard lb_xplane_xp10)
target_link_libraries(lb_xplane_ui_xp11 PUBLIC clipboard lb_xplane_xp11)


if(WIN32)
    find_package(ZLIB REQUIRED)
	find_package(libpng CONFIG REQUIRED)
	target_link_libraries(lb_xplane_ui_xp10 PUBLIC png_static ZLIB::ZLIB)
	target_link_libraries(lb_xplane_ui_xp11 PUBLIC png_static ZLIB::ZLIB)
else()
    if(APPLE)
		FIND_LIBRARY(COCOA_LIBRARY Cocoa)
		FIND_LIBRARY(PNG_LIBRARY NAMES libpng16.a libpng.a)
        target_link_libraries(lb_xplane_ui_xp10 PUBLIC ${COCOA_LIBRARY} ${PNG_LIBRARY} z)
        target_link_libraries(lb_xplane_ui_xp11 PUBLIC ${COCOA_LIBRARY} ${PNG_LIBRARY} z)
	else()
		find_package(PNG REQUIRED)
        target_link_libraries(lb_xplane_ui_xp10 PUBLIC PNG::PNG z)
        target_link_libraries(lb_xplane_ui_xp11 PUBLIC PNG::PNG z)
	endif()
	
endif(WIN32)