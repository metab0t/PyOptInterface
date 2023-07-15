cmake_minimum_required(VERSION 3.15...3.26)

project(pyoptinterface_nb)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(pyoptinterface_core STATIC)
target_sources(pyoptinterface_core PRIVATE
	include/pyoptinterface/core.hpp
	src/core.cpp
)
target_include_directories(pyoptinterface_core PUBLIC include thirdparty)

add_executable(test_main src/main.cpp)
target_include_directories(test_main PUBLIC thirdparty)
target_link_libraries(test_main PUBLIC pyoptinterface_core)

# Build Python extensions
if (SKBUILD)

find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

# We are now ready to compile the actual extension module
nanobind_add_module(
  # Name of the extension
  pyoptinterface_nb_ext

  # Target the stable ABI for Python 3.12+, which reduces
  # the number of binary wheels that must be built. This
  # does nothing on older Python versions
  STABLE_ABI

  # Build libnanobind statically and merge it into the
  # extension (which itself remains a shared library)
  #
  # If your project builds multiple extensions, you can
  # replace this flag by NB_SHARED to conserve space by
  # reusing a shared libnanobind across libraries
  NB_STATIC

  # Source code goes here
  src/pyoptinterface_nb_ext.cpp
)

target_link_libraries(pyoptinterface_nb_ext PUBLIC pyoptinterface_core)

# Install directive for scikit-build
install(TARGETS pyoptinterface_nb_ext LIBRARY DESTINATION pyoptinterface_nb)

else()

# Use Catch2 to build c++ unit test
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
	add_library(gurobi_wrapper STATIC)
	target_sources(gurobi_wrapper PRIVATE
		src/gurobi_wrapper.cpp
	)
	target_link_libraries(gurobi_wrapper PUBLIC pyoptinterface_core)
	target_link_libraries(gurobi_wrapper PUBLIC Gurobi::GurobiC)
endif()