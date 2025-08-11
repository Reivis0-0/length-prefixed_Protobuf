cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

project(protobuf-parser VERSION 1.0.0 DESCRIPTION "Разбор потока length-prefixed Protobuf сообщений")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
find_package(GTest REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src/protocol 
    ${Protobuf_INCLUDE_DIRS}
)

enable_testing()

add_subdirectory(src/protocol)
add_subdirectory(test)