cmake_minimum_required(VERSION 3.15...3.27)

project(pyoptinterface)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(thirdparty/fmt)

set(POI_INSTALL_DIR pyoptinterface/_src)

if(MSVC)
    # Add /MP flag for multi-processor compilation
    add_compile_options(/MP)
endif()

add_library(core STATIC)
target_sources(core PRIVATE
  include/pyoptinterface/core.hpp
  include/pyoptinterface/container.hpp
  include/pyoptinterface/solver_common.hpp
  lib/core.cpp
  lib/cache_model.cpp
  lib/solver_common.cpp
)
target_include_directories(core PUBLIC include thirdparty)
target_link_libraries(core PUBLIC fmt)

# Build Python extensions
find_package(Python 3.8
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

# Solvers

# Gurobi
set(ENABLE_GUROBI ON CACHE BOOL "Enable gurobi?")
if(ENABLE_GUROBI)
  find_package(Gurobi)

  if(NOT Gurobi_FOUND)
    message(FATAL_ERROR "Gurobi not found!")
  endif()

  add_library(gurobi_model STATIC)
  target_sources(gurobi_model PRIVATE
    include/pyoptinterface/gurobi_model.hpp
    lib/gurobi_model.cpp
  )
  target_link_libraries(gurobi_model PUBLIC core)
  target_link_libraries(gurobi_model PUBLIC Gurobi::GurobiC)

  nanobind_add_module(
    gurobi_model_ext

    STABLE_ABI NB_STATIC

    lib/gurobi_model_ext.cpp
    lib/gurobi_model_ext_constants.cpp
  )
  target_link_libraries(gurobi_model_ext PUBLIC gurobi_model)
  install(TARGETS gurobi_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})
endif()

# COPT
set(ENABLE_COPT ON CACHE BOOL "Enable COPT?")
if(ENABLE_COPT)
  find_package(COPT)

  if(NOT COPT_FOUND)
    message(FATAL_ERROR "COPT not found!")
  endif()

  add_library(copt_model STATIC)
  target_sources(copt_model PRIVATE
    include/pyoptinterface/copt_model.hpp
    lib/copt_model.cpp
  )
  target_link_libraries(copt_model PUBLIC core)
  target_link_libraries(copt_model PUBLIC COPT::COPT_C)

  nanobind_add_module(
    copt_model_ext

    STABLE_ABI NB_STATIC

    lib/copt_model_ext.cpp
    lib/copt_model_ext_constants.cpp
  )
  target_link_libraries(copt_model_ext PUBLIC copt_model)
  install(TARGETS copt_model_ext LIBRARY DESTINATION ${POI_INSTALL_DIR})
endif()

add_executable(test_main lib/main.cpp)
target_link_libraries(test_main PUBLIC core gurobi_model)