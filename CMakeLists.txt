cmake_minimum_required(VERSION 3.15...3.27)

project(pyoptinterface_nb)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(pyoptinterface_core STATIC)
target_sources(pyoptinterface_core PRIVATE
  include/pyoptinterface/core.hpp
  include/pyoptinterface/container.hpp
  src/core.cpp
  src/cache_model.cpp
)
target_include_directories(pyoptinterface_core PUBLIC include thirdparty)

# Build Python extensions
find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(
  core_ext

  STABLE_ABI NB_STATIC

  src/core_ext.cpp
)

target_link_libraries(core_ext PUBLIC pyoptinterface_core)

# Install directive for scikit-build
install(TARGETS core_ext LIBRARY DESTINATION pyoptinterface_nb)

# Use Catch2 to build c++ unit test
set(ENABLE_CPP_TEST OFF CACHE BOOL "Enable C++ test with Catch2?")
if(ENABLE_CPP_TEST)
  find_package(Catch2 CONFIG REQUIRED)
  include(CTest)
  include(Catch)

  add_executable(test_basic tests/cpp/basic.cpp)
  target_link_libraries(test_basic PUBLIC pyoptinterface_core)
  target_link_libraries(test_basic PRIVATE Catch2::Catch2WithMain)

  catch_discover_tests(test_basic)
endif()

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
    src/gurobi_model.cpp
  )
  target_link_libraries(gurobi_model PUBLIC pyoptinterface_core)
  target_link_libraries(gurobi_model PUBLIC Gurobi::GurobiC)

  nanobind_add_module(
    gurobi_model_ext

    STABLE_ABI NB_STATIC

    src/gurobi_model_ext.cpp
    src/gurobi_model_ext_constants.cpp
  )
  target_link_libraries(gurobi_model_ext PUBLIC gurobi_model)
  install(TARGETS gurobi_model_ext LIBRARY DESTINATION pyoptinterface_nb)
endif()

add_executable(test_main src/main.cpp)
target_link_libraries(test_main PUBLIC pyoptinterface_core gurobi_model)