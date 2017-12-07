cmake_minimum_required(VERSION 3.1.0)
project(saints)

find_package(Qt5Core)
include_directories("include")

set (SOURCES
    src/ByteIO.cpp
    src/Packfile.cpp
    src/PackfileEntry.cpp
    src/util.cpp)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Qt5::Core)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE True)
