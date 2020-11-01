SET(LIB_SCAN_SRC scan/scan_entity.cpp scan/scan_files.cpp)
SET(LIB_SCAN_HDR scan/scan_entity.h scan/scan_files.h)
SET(LIB_SEARCH_SRC search/allrefs.cpp search/commandref.cpp search/dataref.cpp search/search.cpp)
SET(LIB_SEARCH_HDR search/allrefs.h search/commandref.h search/dataref.h search/ref.h search/search.h)
SET(LIB_UTIL_SRC util/string_util.cpp)
SET(LIB_UTIL_HDR util/visit_backport.h util/string_util.h)

SET(LIB_HDR ${LIB_SCAN_HDR} ${LIB_SEARCH_HDR} ${LIB_UTIL_HDR})
SET(LIB_SRC ${LIB_SCAN_SRC} ${LIB_SEARCH_SRC} ${LIB_UTIL_SRC})

SET(LIB_SRC ${LIB_SRC} ${LIB_HDR})
LIST(SORT LIB_SRC)

add_library(drt STATIC ${LIB_SRC} ${LIB_HDR})

find_package(Boost 1.54.0 REQUIRED COMPONENTS filesystem system iostreams regex)
target_link_libraries(drt PUBLIC Boost::filesystem Boost::system Boost::iostreams)
target_include_directories(drt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# define the minimum X-Plane SDK version that gives us the functions we need for the API
add_xplane_sdk_definitions(drt 210)

if(UNIX AND NOT APPLE)
    set_property(TARGET drt PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
