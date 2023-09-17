function(configure_defaults)
    set(DISARRAY_COMPILE_PROFILE OFF CACHE BOOL "Compile time profilation")
    set(DISARRAY_LOG_ALLOCATIONS OFF CACHE BOOL "Log all allocations")
    set(DISARRAY_USE_VULKAN ON CACHE BOOL "Use vulkan over some other API")
    set(DISARRAY_BUILD_BENCHMARKS OFF CACHE BOOL "Build some benchmarks!")
    set(DISARRAY_BUILD_TESTS ON CACHE BOOL "Build tests")

    # find_package(OpenMP REQUIRED)
endfunction(configure_defaults)
