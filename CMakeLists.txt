cmake_minimum_required(VERSION 3.15...3.27)

project(pyoptinterface)

set(CMAKE_CXX_STANDARD 20)
# Linux: -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# we need to ensure all shared libraries can look up its own directory to load other shared libraries
# this is very important for CppAD users because we extract its thread_alloc as a shared library
function(set_rpath target)
    if(APPLE)
        set(CMAKE_MACOSX_RPATH 1)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
    elseif(WIN32)
    endif()
endfunction()

if(MSVC)
    # Add /MP flag for multi-processor compilation
    add_compile_options(/MP)
    add_compile_options(/utf-8)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
        # use AVX2
        add_compile_options(/arch:AVX2)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        # add_compile_options(-march=haswell)
    endif()
endif()

add_subdirectory(thirdparty/fmt)
add_subdirectory(thirdparty/cppad)

set(POI_INSTALL_DIR ${SKBUILD_PLATLIB_DIR}/pyoptinterface/_src)
# install the DLL of cppad
if(MSVC)
    install(TARGETS cppad_thread_alloc RUNTIME DESTINATION ${POI_INSTALL_DIR})
else()
    install(TARGETS cppad_thread_alloc LIBRARY DESTINATION ${POI_INSTALL_DIR})
endif()

add_library(core STATIC)
target_sources(core PRIVATE
  include/pyoptinterface/core.hpp
  include/pyoptinterface/container.hpp
  include/pyoptinterface/dylib.hpp
  include/pyoptinterface/solver_common.hpp
  lib/core.cpp
  lib/cache_model.cpp
)
target_include_directories(core PUBLIC include thirdparty)
target_link_libraries(core PUBLIC fmt)

add_library(nlcore STATIC)
target_sources(nlcore PRIVATE
  include/pyoptinterface/nlcore.hpp
  lib/nlcore.cpp
)
target_link_libraries(nlcore PUBLIC core cppad)

# Build Python extensions
find_package(Python ${PYTHON_VERSION}
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(
  core_ext

  STABLE_ABI NB_STATIC

  lib/core_ext.cpp
)
target_link_libraries(core_ext PUBLIC core)
install(TARGETS core_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

nanobind_add_module(
  nlcore_ext

  STABLE_ABI NB_STATIC

  lib/nlcore_ext.cpp
)
target_link_libraries(nlcore_ext PUBLIC nlcore)
install(TARGETS nlcore_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# Solvers

# Gurobi
add_library(gurobi_model STATIC)
target_sources(gurobi_model PRIVATE
include/pyoptinterface/gurobi_model.hpp
lib/gurobi_model.cpp
)
target_link_libraries(gurobi_model PUBLIC core)

nanobind_add_module(
gurobi_model_ext

STABLE_ABI NB_STATIC

lib/gurobi_model_ext.cpp
lib/gurobi_model_ext_constants.cpp
)
target_link_libraries(gurobi_model_ext PUBLIC gurobi_model)
install(TARGETS gurobi_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# COPT
add_library(copt_model STATIC)
target_sources(copt_model PRIVATE
include/pyoptinterface/copt_model.hpp
lib/copt_model.cpp
)
target_link_libraries(copt_model PUBLIC core)

nanobind_add_module(
copt_model_ext

STABLE_ABI NB_STATIC

lib/copt_model_ext.cpp
lib/copt_model_ext_constants.cpp
)
target_link_libraries(copt_model_ext PUBLIC copt_model)
install(TARGETS copt_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# MOSEK
add_library(mosek_model STATIC)
target_sources(mosek_model PRIVATE
include/pyoptinterface/mosek_model.hpp
lib/mosek_model.cpp
)
target_link_libraries(mosek_model PUBLIC core)

nanobind_add_module(
mosek_model_ext

STABLE_ABI NB_STATIC

lib/mosek_model_ext.cpp
lib/mosek_model_ext_constants.cpp
)
target_link_libraries(mosek_model_ext PUBLIC mosek_model)
install(TARGETS mosek_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# HiGHS
add_library(highs_model STATIC)
target_sources(highs_model PRIVATE
include/pyoptinterface/highs_model.hpp
lib/highs_model.cpp
)
target_include_directories(highs_model PUBLIC thirdparty/solvers/highs)
target_link_libraries(highs_model PUBLIC core)
# target_link_libraries(highs_model PUBLIC HiGHS::HiGHS)

nanobind_add_module(
highs_model_ext

STABLE_ABI NB_STATIC

lib/highs_model_ext.cpp
lib/highs_model_ext_constants.cpp
)
target_link_libraries(highs_model_ext PUBLIC highs_model)
install(TARGETS highs_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# IPOPT
add_library(ipopt_model STATIC)
target_sources(ipopt_model PRIVATE
  include/pyoptinterface/ipopt_model.hpp
  lib/ipopt_model.cpp
)
target_link_libraries(ipopt_model PUBLIC nlcore)

nanobind_add_module(
  ipopt_model_ext

  STABLE_ABI NB_STATIC

  lib/ipopt_model_ext.cpp
)
target_link_libraries(ipopt_model_ext PUBLIC ipopt_model)
install(TARGETS ipopt_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})

# Set RPATH
set_rpath(nlcore_ext)
set_rpath(ipopt_model_ext)

# stub
nanobind_add_stub(
  core_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.core_ext
  OUTPUT ${POI_INSTALL_DIR}/core_ext.pyi
)

nanobind_add_stub(
  nlcore_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.nlcore_ext
  OUTPUT ${POI_INSTALL_DIR}/nlcore_ext.pyi
)

nanobind_add_stub(
  gurobi_model_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.gurobi_model_ext
  OUTPUT ${POI_INSTALL_DIR}/gurobi_model_ext.pyi
)

nanobind_add_stub(
  copt_model_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.copt_model_ext
  OUTPUT ${POI_INSTALL_DIR}/copt_model_ext.pyi
)

nanobind_add_stub(
  mosek_model_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.mosek_model_ext
  OUTPUT ${POI_INSTALL_DIR}/mosek_model_ext.pyi
)

nanobind_add_stub(
  highs_model_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.highs_model_ext
  OUTPUT ${POI_INSTALL_DIR}/highs_model_ext.pyi
)

nanobind_add_stub(
  ipopt_model_ext_stub
  INSTALL_TIME
  MODULE pyoptinterface._src.ipopt_model_ext
  OUTPUT ${POI_INSTALL_DIR}/ipopt_model_ext.pyi
)

set(ENABLE_TEST_MAIN OFF CACHE BOOL "Enable test c++ function with a main.cpp")
if(ENABLE_TEST_MAIN)
  add_executable(test_main lib/main.cpp)
  target_link_libraries(test_main PUBLIC core gurobi_model copt_model mosek_model highs_model)
endif()