cmake_minimum_required(VERSION 3.22)

project(App CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/argparse)

set(APP_SOURCES
        include/panels/ScenePanel.hpp
        src/panels/ScenePanel.cpp
        include/panels/ExecutionStatisticsPanel.hpp
        src/panels/ExecutionStatisticsPanel.cpp
        include/panels/DirectoryContentPanel.hpp
        src/panels/DirectoryContentPanel.cpp
        include/ClientLayer.hpp
        src/ClientLayer.cpp
        include/panels/StatisticsPanel.hpp
        src/panels/StatisticsPanel.cpp
)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
        ${APP_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine spdlog::spdlog argparse::argparse)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_FORCE_RADIANS)

default_compile_flags()

if(DISARRAY_COMPILER STREQUAL "MSVC")
        target_compile_options(${PROJECT_NAME} PRIVATE /wd4267)
endif()

if (EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json AND CMAKE_EXPORT_COMPILE_COMMANDS)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}
	)
endif()

if(DISARRAY_COMPILER STREQUAL "Clang" OR DISARRAY_COMPILER STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-compare)
endif()
