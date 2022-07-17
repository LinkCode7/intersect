cmake_minimum_required(VERSION 3.19)
project(intersect)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC
       ${PROJECT_SOURCE_DIR}/*.cpp
       ${PROJECT_SOURCE_DIR}/*.h
       ${PROJECT_SOURCE_DIR}/Common/*.cpp
       ${PROJECT_SOURCE_DIR}/Common/*.h
       ${PROJECT_SOURCE_DIR}/RangeBound/*.cpp
       ${PROJECT_SOURCE_DIR}/RangeBound/*.h
       ${PROJECT_SOURCE_DIR}/SQLite/*.cpp
       ${PROJECT_SOURCE_DIR}/SQLite/*.c
       ${PROJECT_SOURCE_DIR}/SQLite/*.h
       ${PROJECT_SOURCE_DIR}/Testing/*.cpp
       ${PROJECT_SOURCE_DIR}/Testing/*.h)

#打印SRC中列表信息
message(STATUS "SRC = ${SRC}")

add_executable(${PROJECT_NAME} ${SRC})
