cmake_minimum_required(VERSION 3.8...3.26)

project(FMT CXX)

file(GLOB FMT_HEADERS include/fmt/*.h)
set(FMT_SOURCES src/format.cc src/os.cc)

add_library(fmt STATIC)
target_sources(fmt PRIVATE
  ${FMT_HEADERS}
  ${FMT_SOURCES}
)
target_include_directories(fmt PUBLIC include)
