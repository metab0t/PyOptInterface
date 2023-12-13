# Once done this will define
#  CPOT_FOUND - System has COPT
#  Targets:
#       COPT::COPT_C   - only the C interface
#       COPT::COPT_CXX - C and C++ interface

find_path(COPT_HOME
          NAMES include/copt.h
          PATHS
          $ENV{COPT_HOME}
          NO_DEFAULT_PATH # avoid finding /usr
          )

find_path(COPT_INCLUDE_DIR
    NAMES copt.h
    HINTS
    "${COPT_HOME}/include"
    )
mark_as_advanced(COPT_INCLUDE_DIR)

set(COPT_BIN_DIR "${COPT_HOME}/bin")
set(COPT_LIB_DIR "${COPT_HOME}/lib")

if (WIN32)
    find_file(COPT_LIBRARY
        NAMES "copt.dll"
        PATHS
        ${COPT_BIN_DIR}
    )
    find_library(COPT_IMPLIB
        NAMES "copt"
        PATHS
        ${COPT_LIB_DIR}
    )
    mark_as_advanced(COPT_IMPLIB)
    find_file(COPT_CXX_LIBRARY
        NAMES "copt_cpp.dll"
        PATHS
        ${COPT_BIN_DIR}
    )
    find_library(COPT_CXX_IMPLIB
        NAMES "copt_cpp"
        PATHS
        ${COPT_LIB_DIR}
    )
    mark_as_advanced(COPT_CXX_IMPLIB)
else ()
    find_library(COPT_LIBRARY
        NAMES "copt"
        PATHS
        ${COPT_LIB_DIR}
    )
    find_library(COPT_CXX_LIBRARY
        NAMES "copt_cpp"
        PATHS
        ${COPT_LIB_DIR}
    )
endif()
mark_as_advanced(COPT_LIBRARY)
mark_as_advanced(COPT_CXX_LIBRARY)

if(COPT_LIBRARY)
    add_library(COPT::COPT_C SHARED IMPORTED)
    target_include_directories(COPT::COPT_C INTERFACE ${COPT_INCLUDE_DIR})
    set_target_properties(COPT::COPT_C PROPERTIES IMPORTED_LOCATION ${COPT_LIBRARY})
    if (COPT_IMPLIB)
        set_target_properties(COPT::COPT_C PROPERTIES IMPORTED_IMPLIB ${COPT_IMPLIB})
    endif()
endif()

if(COPT_LIBRARY AND COPT_CXX_LIBRARY)
    add_library(COPT::COPT_CXX SHARED IMPORTED)
    target_include_directories(COPT::COPT_CXX INTERFACE ${COPT_INCLUDE_DIR})
    set_target_properties(COPT::COPT_CXX PROPERTIES IMPORTED_LOCATION ${COPT_CXX_LIBRARY})
    if (COPT_CXX_IMPLIB)
        set_target_properties(COPT::COPT_CXX PROPERTIES IMPORTED_IMPLIB ${COPT_CXX_IMPLIB})
    endif()
    target_link_libraries(COPT::COPT_CXX INTERFACE COPT::COPT_C)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(COPT DEFAULT_MSG COPT_LIBRARY COPT_INCLUDE_DIR)
