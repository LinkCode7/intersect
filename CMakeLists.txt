cmake_minimum_required(VERSION 3.12)
project(intersect)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(THIS_PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# test boost.geometry.spatial_indexes.rtree
set(TEST_BOOST_RTREE ON)

if(TEST_BOOST_RTREE)
set(Boost_INCLUDE_DIR "..//boost_1_79_0")
set(Boost_LIBRARY_DIRS "..//boost_1_79_0//stage//lib")
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(BOOST_ALL_DYN_LINK           ON)
add_compile_definitions(TEST_BOOST_RTREE)
endif()

# 工程文件
aux_source_directory(./ SRCS)
aux_source_directory(./bound SRCS)
aux_source_directory(./common SRCS)
aux_source_directory(./sqlite SRCS)
aux_source_directory(./test SRCS)

#打印SRC中列表信息
message(STATUS "SRC = ${SRCS}")

add_executable(${PROJECT_NAME} ${SRCS})

if(TEST_BOOST_RTREE)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()