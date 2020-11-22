
add_subdirectory(clipboard)
add_subdirectory(json EXCLUDE_FROM_ALL)
add_subdirectory(xplane_sdk)

add_subdirectory(glew)
add_subdirectory(mio)

add_subdirectory(lb_common)

add_subdirectory(lb_xplane) # depends on xplane_sdk, glew, and lb_filesystem (lb_common)
add_subdirectory(lb_xplane_ui) # depends on lb_xplane
