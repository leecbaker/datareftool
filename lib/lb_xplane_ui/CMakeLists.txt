FILE(GLOB LB_XPLANE_UI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/container/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.cpp")
FILE(GLOB LB_XPLANE_UI_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/container/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.h")

add_library(lb_xplane_ui STATIC ${LB_XPLANE_UI_SRC} ${LB_XPLANE_UI_HDR})
target_include_directories(lb_xplane_ui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
	TARGET_COMPILE_OPTIONS(lb_xplane_ui PRIVATE "/MT$<$<CONFIG:Debug>:d>")
endif(WIN32)

if(UNIX AND NOT APPLE)
    set_property(TARGET lb_xplane_ui PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(lb_xplane_ui PUBLIC clipboard lb_xplane)