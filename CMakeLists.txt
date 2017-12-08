cmake_minimum_required(VERSION 3.1.0)
project(saints)

find_package(Qt5Core REQUIRED)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(include)

set(SOURCES
    src/ByteIO.cpp
    src/Packfile.cpp
    src/PackfileEntry.cpp
    src/util.cpp)

option(BUILD_SHARED_LIBS "Build library as a shared object")
add_library(saints ${SOURCES})
target_link_libraries(saints Qt5::Core)
target_link_libraries(saints ${ZLIB_LIBRARIES})
set_property(TARGET saints PROPERTY POSITION_INDEPENDENT_CODE True)

install(TARGETS saints EXPORT saintsTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
install(DIRECTORY include/Saints DESTINATION include)

export(EXPORT saintsTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/saints/saintsTargets.cmake"
    NAMESPACE Upstream::
)
configure_file(cmake/saintsConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/saints/saintsConfig.cmake"
    COPYONLY
)

set(ConfigPackageLocation lib/cmake/saints)
install(EXPORT saintsTargets
    FILE saintsTargets.cmake
    NAMESPACE Upstream::
    DESTINATION ${ConfigPackageLocation}
)
install(FILES cmake/saintsConfig.cmake DESTINATION ${ConfigPackageLocation})
