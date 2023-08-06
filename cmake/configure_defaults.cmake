function(configure_defaults)
    set(GLFW_INSTALL OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(BUILD_SHARED_LIBS OFF)
    option(DISARRAY_LOG_ALLOCATIONS "Log all allocations" OFF)

    # find_package(OpenMP REQUIRED)
endfunction(configure_defaults)
