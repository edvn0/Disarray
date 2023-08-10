cmake_minimum_required(VERSION 3.22)

project(DisarrayTests CXX)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

if (WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

add_executable(${PROJECT_NAME} graphics/batch_renderer_test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Disarray::Engine GTest::gtest GTest::gtest_main Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator imgui tinyobjloader stb_image thread-pool EnTT::EnTT fmt::fmt)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
