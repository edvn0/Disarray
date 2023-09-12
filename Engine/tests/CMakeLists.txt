cmake_minimum_required(VERSION 3.22)

project(DisarrayTests CXX)

if (WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

add_executable(${PROJECT_NAME} main.cpp scene/serialise_compare_test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE libtinyfiledialogs ApprovalTests::ApprovalTests Disarray::Engine GTest::gtest magic_enum::magic_enum imguizmo nlohmann_json::nlohmann_json Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator imgui tinyobjloader stb_image thread-pool EnTT::EnTT fmt::fmt)
default_compile_flags()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
