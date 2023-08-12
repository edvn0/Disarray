cmake_minimum_required(VERSION 3.22)

project(DisarrayTests CXX)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
FetchContent_Declare(ApprovalTests
        GIT_REPOSITORY https://github.com/approvals/ApprovalTests.cpp.git
        GIT_TAG master)

FetchContent_MakeAvailable(ApprovalTests)

if (WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

add_executable(${PROJECT_NAME} main.cpp scene/serialise_compare_test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ApprovalTests::ApprovalTests Disarray::Engine GTest::gtest magic_enum::magic_enum nlohmann_json::nlohmann_json Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator imgui tinyobjloader stb_image thread-pool EnTT::EnTT fmt::fmt)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
