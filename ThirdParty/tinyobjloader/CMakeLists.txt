cmake_minimum_required(VERSION 3.22)

project(tinyobjloader CXX)

add_library(${PROJECT_NAME} STATIC src/tinyobjloader.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
