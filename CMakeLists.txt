cmake_minimum_required(VERSION 3.19)
project(intersect)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(./ SRCS)
aux_source_directory(./Common SRCS)
aux_source_directory(./RangeBound SRCS)
aux_source_directory(./SQLite SRCS)
aux_source_directory(./Testing SRCS)

#打印SRC中列表信息
message(STATUS "SRC = ${SRCS}")

add_executable(${PROJECT_NAME} ${SRCS})
