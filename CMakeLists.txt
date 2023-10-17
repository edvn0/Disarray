cmake_minimum_required(VERSION 3.22)

project(Disarray CXX C)

include("cmake/detect_system.cmake")
include("cmake/default_compile_flags.cmake")
include("cmake/configure_defaults.cmake")
configure_defaults()
detect_os()
detect_compiler()

if(DISARRAY_BUILD_TESTS)
  enable_testing()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

find_package(Vulkan REQUIRED)
find_program(CLANG_FORMAT "clang-format")
include(FetchContent)

add_subdirectory(Implementations)
add_subdirectory(Engine)
add_subdirectory(ThirdParty)
add_subdirectory(App)

if(DISARRAY_BUILD_BENCHMARKS)
    add_subdirectory(Benchmarks)
endif()

if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  option(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR} OUTPUT OUT)
  message(
    STATUS
      "Copying ${CMAKE_BINARY_DIR}/compile_commands.json to ${CMAKE_SOURCE_DIR}"
  )
endif()

