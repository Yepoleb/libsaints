# - Try to find LZ4
# Once done this will define
#  LZ4_FOUND - System has LZ4
#  LZ4_INCLUDE_DIRS - The LZ4 include directories
#  LZ4_LIBRARIES - The libraries needed to use LZ4

find_package(PkgConfig)
pkg_check_modules(PC_LZ4 QUIET liblz4)

find_path(LZ4_INCLUDE_DIR lz4.h
    HINTS ${PC_LZ4_INCLUDEDIR} ${PC_LZ4_INCLUDE_DIRS})

find_library(LZ4_LIBRARY NAMES lz4 liblz4
    HINTS ${PC_LZ4_LIBDIR} ${PC_LZ4_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LZ4_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_LIBRARY LZ4_INCLUDE_DIR)

mark_as_advanced(LZ4_INCLUDE_DIR LZ4_LIBRARY)

set(LZ4_LIBRARIES ${LZ4_LIBRARY})
set(LZ4_INCLUDE_DIRS ${LZ4_INCLUDE_DIR})
