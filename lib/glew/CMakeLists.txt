add_library(glew STATIC glew.c)
target_compile_definitions(glew PUBLIC GLEW_STATIC)
target_include_directories(glew INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX AND NOT APPLE)
    set_property(TARGET glew PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

if(APPLE)
    find_library(GL_LIBRARY OpenGL)
    target_link_libraries(glew PUBLIC ${GL_LIBRARY})
endif()