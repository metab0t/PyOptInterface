# Once done this will define
#  MOSEK_FOUND - System has MOSEK
#  Targets:
#       MOSEK::MOSEK_C   - only the C interface

find_path(MOSEK_HOME
          NAMES h/mosek.h
          PATHS
          $ENV{MOSEK_10_1_BINDIR}/..
          NO_DEFAULT_PATH
          )

find_path(MOSEK_INCLUDE_DIR
    NAMES mosek.h
    HINTS
    "${MOSEK_HOME}/h"
    )
mark_as_advanced(MOSEK_INCLUDE_DIR)

set(MOSEK_BIN_DIR "${MOSEK_HOME}/bin")
set(MOSEK_LIB_DIR "${MOSEK_HOME}/bin")

if (WIN32)
    find_file(MOSEK_LIBRARY
        NAMES "mosek64_10_1.dll"
        PATHS
        ${MOSEK_BIN_DIR}
    )
    find_library(MOSEK_IMPLIB
        NAMES "mosek64_10_1.lib"
        PATHS
        ${MOSEK_LIB_DIR}
    )
    mark_as_advanced(MOSEK_IMPLIB)
else ()
    find_library(MOSEK_LIBRARY
        NAMES "mosek64_10_1"
        PATHS
        ${MOSEK_LIB_DIR}
    )
endif()
mark_as_advanced(MOSEK_LIBRARY)

if(MOSEK_LIBRARY)
    add_library(MOSEK::MOSEK_C SHARED IMPORTED)
    target_include_directories(MOSEK::MOSEK_C INTERFACE ${MOSEK_INCLUDE_DIR})
    set_target_properties(MOSEK::MOSEK_C PROPERTIES IMPORTED_LOCATION ${MOSEK_LIBRARY})
    if (MOSEK_IMPLIB)
        set_target_properties(MOSEK::MOSEK_C PROPERTIES IMPORTED_IMPLIB ${MOSEK_IMPLIB})
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MOSEK DEFAULT_MSG MOSEK_LIBRARY MOSEK_INCLUDE_DIR)
