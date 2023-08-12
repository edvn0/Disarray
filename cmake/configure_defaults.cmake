function(configure_defaults)
    set(GLFW_INSTALL OFF CACHE BOOL "GLFW" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "GLFW" FORCE)
    set(DISARRAY_COMPILE_PROFILE OFF CACHE BOOL "Compile time profilation")
    set(DISARRAY_BUILD_TESTS ON CACHE BOOL "Build tests")
    set(DISARRAY_LOG_ALLOCATIONS OFF CACHE BOOL "Log all allocations")

    # find_package(OpenMP REQUIRED)
endfunction(configure_defaults)
