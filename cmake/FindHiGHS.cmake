# Once done this will define
#  HiGHS_FOUND - System has HiGHS
#  Targets:
#       HiGHS::HiGHS

find_path(HiGHS_HOME
          NAMES include/highs/Highs.h
          HINTS
          $ENV{HiGHS_HOME}
          )

find_path(HiGHS_INCLUDE_DIR
    NAMES Highs.h
    HINTS
    "${HiGHS_HOME}/include/highs"
    )
mark_as_advanced(HiGHS_INCLUDE_DIR)

set(HiGHS_BIN_DIR "${HiGHS_HOME}/bin")
set(HiGHS_LIB_DIR "${HiGHS_HOME}/lib")

if (WIN32)
    find_file(HiGHS_LIBRARY
        NAMES "highs.dll" "libhighs.dll"
        HINTS
        ${HiGHS_BIN_DIR}
    )
    find_library(HiGHS_IMPLIB
        NAMES "highs.lib"
        HINTS
        ${HiGHS_LIB_DIR}
    )
    mark_as_advanced(HiGHS_IMPLIB)
else ()
    find_library(HiGHS_LIBRARY
        NAMES "highs"
        PATHS
        ${HiGHS_LIB_DIR}
    )
endif()
mark_as_advanced(HiGHS_LIBRARY)

if(HiGHS_LIBRARY)
    add_library(HiGHS::HiGHS SHARED IMPORTED)
    target_include_directories(HiGHS::HiGHS INTERFACE ${HiGHS_INCLUDE_DIR})
    set_target_properties(HiGHS::HiGHS PROPERTIES IMPORTED_LOCATION ${HiGHS_LIBRARY})
    if (HiGHS_IMPLIB)
        set_target_properties(HiGHS::HiGHS PROPERTIES IMPORTED_IMPLIB ${HiGHS_IMPLIB})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HiGHS DEFAULT_MSG HiGHS_LIBRARY HiGHS_INCLUDE_DIR)
