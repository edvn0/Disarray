function(default_compile_flags)
	if(DISARRAY_COMPILER MATCHES "Clang")
		target_compile_options(
			${PROJECT_NAME} PRIVATE -Werror -Wno-defaulted-function-deleted -Wall -Wno-nullability-completeness
			-Wno-unused-variable -Wno-unknown-attributes -Wno-deprecated-declarations)

		if(DISARRAY_OS STREQUAL "Linux")
			find_package(TBB REQUIRED)
			target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
			target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
		endif()

		if(DISARRAY_OS STREQUAL "Windows")
			target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
		endif()

		if(DISARRAY_COMPILER STREQUAL "AppleClang")
			target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_MACOS)
		endif()
	elseif(DISARRAY_COMPILER STREQUAL "GNU")
		find_package(TBB REQUIRED)

		target_compile_options(
			${PROJECT_NAME} PRIVATE -Werror -Wno-defaulted-function-deleted -Wall -Wno-nullability-completeness
			-Wno-unused-variable -Wno-unknown-attributes -Wno-deprecated-declarations)
		target_link_libraries(${PROJECT_NAME} PRIVATE tbb)
		target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_LINUX)
	elseif(DISARRAY_COMPILER STREQUAL "MSVC")
		target_compile_options(
			${PROJECT_NAME}
			PRIVATE /MP
			/W4
			/WX
			/wd4244
			/wd4100
			/wd4189
			/wd4127
			/wd4324
			/wd4201
			/wd4702)
		target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_WINDOWS)
	endif()

	if(DISARRAY_LOG_ALLOCATIONS)
		target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_ALLOCATIONS)
	endif()

	if()
	endif()

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		target_compile_definitions(${PROJECT_NAME} PRIVATE IS_RELEASE)
	endif()

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND DISARRAY_OS STREQUAL "Windows")
		target_compile_definitions(${PROJECT_NAME} PRIVATE USE_VALIDATION_LAYERS)
	endif()

	if(DISARRAY_USE_VULKAN)
		target_compile_definitions(${PROJECT_NAME} PRIVATE DISARRAY_VULKAN)
	endif()
endfunction()
