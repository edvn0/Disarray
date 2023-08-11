function(default_compile_flags)
    if (DISARRAY_COMPILER MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)

        if (DISARRAY_OS STREQUAL "Linux")
            find_package(TBB REQUIRED)
            target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
            target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
        endif ()

        if (DISARRAY_COMPILER STREQUAL "AppleClang")
            target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_MACOS)
        endif ()
    elseif (DISARRAY_COMPILER STREQUAL "GNU")
        find_package(TBB REQUIRED)

        # VMA, STB ignores - -Wno-nullability-completeness -Wno-unused-variable
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wno-nullability-completeness -Wno-unused-variable)
        target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
        target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
    elseif (DISARRAY_COMPILER STREQUAL "MSVC")
        # VMA, STB ignores - /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201
        # [[noreturn]] - /wd4702
        target_compile_options(${PROJECT_NAME} PRIVATE /MP /W4 /WX /wd4244 /wd4100 /wd4189 /wd4127 /wd4324 /wd4201 /wd4702)
        target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
    endif ()

    if (DISARRAY_LOG_ALLOCATIONS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_ALLOCATIONS)
    endif ()

    if (NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        target_compile_definitions(${PROJECT_NAME} PRIVATE IS_DEBUG)
    endif ()
endfunction()