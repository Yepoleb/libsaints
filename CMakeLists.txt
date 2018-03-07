cmake_minimum_required(VERSION 3.1.0)
project(saints)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Qt5Core REQUIRED)
find_package(crosstex REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LZ4 REQUIRED)

set(SOURCES
    src/ByteIO.cpp
    src/Packfile.cpp
    src/PackfileEntry.cpp
    src/DDSFile.cpp
    src/PegFile.cpp
    src/PegEntry.cpp
    src/TGAFile.cpp
    src/util.cpp)

option(BUILD_SHARED_LIBS "Build library as a shared object")
add_library(saints ${SOURCES})
target_link_libraries(saints PUBLIC Qt5::Core)
target_link_libraries(saints PRIVATE Upstream::crosstex)
target_link_libraries(saints PRIVATE ${ZLIB_LIBRARIES})
target_link_libraries(saints PRIVATE ${LZ4_LIBRARIES})
target_include_directories(saints PRIVATE ${ZLIB_INCLUDE_DIRS})
target_include_directories(saints PRIVATE ${LZ4_INCLUDE_DIRS})
target_include_directories(saints PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
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
