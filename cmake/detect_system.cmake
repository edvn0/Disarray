function(detect_os)
    if (WIN32)
        set(DISARRAY_OS "Windows" PARENT_SCOPE)
    endif ()

    if (APPLE)
        set(DISARRAY_OS "MacOS" PARENT_SCOPE)
    endif ()

    if (UNIX AND NOT APPLE)
        set(DISARRAY_OS "Linux" PARENT_SCOPE)
    endif ()
endfunction()

function(detect_compiler)
    if (MSVC)
        set(DISARRAY_COMPILER "MSVC" PARENT_SCOPE)
    endif ()

    if (MINGW)
        set(DISARRAY_COMPILER "MinGW" PARENT_SCOPE)
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(DISARRAY_COMPILER "Clang" PARENT_SCOPE)
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(DISARRAY_COMPILER "GNU" PARENT_SCOPE)
    endif ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(DISARRAY_COMPILER "AppleClang" PARENT_SCOPE)
    endif()
endfunction()
