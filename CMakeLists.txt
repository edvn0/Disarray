cmake_minimum_required(VERSION 3.25)

project(Disarray CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

include("cmake/configure_defaults.cmake")
configure_defaults()

add_subdirectory(Engine)
add_subdirectory(ThirdParty)
add_subdirectory(App)

