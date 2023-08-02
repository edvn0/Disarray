cmake_minimum_required(VERSION 3.22)

project(Disarray CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

find_package(Vulkan REQUIRED)
find_program(CLANG_FORMAT "clang-format")

include("cmake/configure_defaults.cmake")
configure_defaults()

add_subdirectory(Engine)
add_subdirectory(ThirdParty)
add_subdirectory(App)


set(ALL_SOURCES "")
list(APPEND ALL_SOURCES ${ENGINE_SOURCES} ${APP_SOURCES})

if (CLANG_FORMAT)
    set(FORMATTABLE_FILES ${ALL_SOURCES})

    add_custom_target(
            "clang-format-${PROJECT_NAME}" COMMAND ${CLANG_FORMAT} -i -style=file
            ${FORMATTABLE_FILES})
endif()
