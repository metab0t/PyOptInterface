cmake_minimum_required(VERSION 3.8...3.26)

project(CPPAD CXX)

add_library(cppad_thread_alloc SHARED)
target_sources(cppad_thread_alloc PRIVATE
  include/cppad/utility/thread_alloc.cpp
)
target_include_directories(cppad_thread_alloc PUBLIC include)
target_compile_definitions(cppad_thread_alloc PRIVATE THREAD_ALLOC_DLL)

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
target_link_libraries(cppad PUBLIC cppad_thread_alloc)