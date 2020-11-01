

FILE(GLOB UI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp")
FILE(GLOB UI_HDR "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.h")
FILE(GLOB PLUGIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB PLUGIN_HDR "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_xplane_plugin(datareftool_xp10 210 ${PLUGIN_SRC} ${PLUGIN_HDR} ${UI_SRC} ${UI_HDR})
add_xplane_plugin(datareftool 301 ${PLUGIN_SRC} ${PLUGIN_HDR} ${UI_SRC} ${UI_HDR})

target_link_libraries(datareftool_xp10 PUBLIC lb_xplane_xp10 lb_xplane_ui_xp10 nlohmann_json drt)
target_link_libraries(datareftool PUBLIC lb_xplane_xp11 lb_xplane_ui_xp11 nlohmann_json drt)