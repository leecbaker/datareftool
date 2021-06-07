

FILE(GLOB DRT_CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
FILE(GLOB DRT_CLIENT_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_xplane_plugin(drt_client 200 ${DRT_CLIENT_SRC} ${DRT_CLIENT_HDR})

target_link_libraries(drt_client PUBLIC lb_xplane_xp10)