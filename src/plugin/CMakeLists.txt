

FILE(GLOB UI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp")
FILE(GLOB UI_HDR "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.h")
FILE(GLOB PLUGIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB PLUGIN_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_xplane_plugin(datareftool2 301 ${PLUGIN_SRC} ${PLUGIN_HDR} ${UI_SRC} ${UI_HDR})

target_link_libraries(datareftool2 PUBLIC lb_xplane lb_xplane_ui nlohmann_json drt)