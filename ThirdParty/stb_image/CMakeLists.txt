cmake_minimum_required(VERSION 3.22)

project(stb_image C)

add_library(${PROJECT_NAME} STATIC src/stb_image.c)
target_include_directories(${PROJECT_NAME} PUBLIC include)
