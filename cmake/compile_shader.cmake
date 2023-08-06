function(compile_shaders)
    set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/App/Assets/Shaders)
    set(SHADER_BINARY_DIR ${PROJECT_BINARY_DIR}/Assets/Shaders)

    file(GLOB_RECURSE SHADERS
            ${SHADER_SOURCE_DIR}/*.vert
            ${SHADER_SOURCE_DIR}/*.frag
            ${SHADER_SOURCE_DIR}/*.comp
            ${SHADER_SOURCE_DIR}/*.geom
            ${SHADER_SOURCE_DIR}/*.tesc
            ${SHADER_SOURCE_DIR}/*.tese
            ${SHADER_SOURCE_DIR}/*.mesh
            ${SHADER_SOURCE_DIR}/*.task
            ${SHADER_SOURCE_DIR}/*.rgen
            ${SHADER_SOURCE_DIR}/*.rchit
            ${SHADER_SOURCE_DIR}/*.rmiss)

    add_custom_command(
            COMMAND
            ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
            OUTPUT ${SHADER_BINARY_DIR}
            COMMENT "Creating ${SHADER_BINARY_DIR}"
    )

    if(${Vulkan_glslangValidator_FOUND})
        foreach(source IN LISTS SHADERS)
            get_filename_component(FILENAME ${source} NAME)
            add_custom_command(
                    COMMAND
                    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
                    -V ${source} -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
                    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
                    DEPENDS ${source} ${SHADER_BINARY_DIR}
                    COMMENT "Compiling ${FILENAME}"
            )
            list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
        endforeach()

        add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
        set_source_files_properties(shaders PROPERTIES SYMBOLIC 1)
    else()
        message(STATUS "Could not find glslangValidator.")
    endif()
endfunction()
