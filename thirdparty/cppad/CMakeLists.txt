cmake_minimum_required(VERSION 3.8...3.26)

project(CPPAD CXX)

file(GLOB CPPAD_HEADERS include/cppad/*.hpp)
set(CPPAD_SOURCES
src/cpp_graph_op.cpp
src/temp_file.cpp
)

add_library(cppad STATIC)
target_sources(cppad PRIVATE
  ${CPPAD_HEADERS}
  ${CPPAD_SOURCES}
)
target_include_directories(cppad PUBLIC include)
