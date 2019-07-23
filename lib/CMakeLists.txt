add_library(glew STATIC glew/glew.c)
target_compile_definitions(glew PUBLIC GLEW_STATIC)
target_include_directories(glew INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glew)

if(UNIX AND NOT APPLE)
    set_property(TARGET glew PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)