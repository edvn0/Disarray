function(copy_directory_to_binary BASE_FOLDER FOLDER_NAMES)
    set(TGT ${PROJECT_NAME})
    add_custom_command(
            TARGET ${TGT} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${TGT}>/${BASE_FOLDER}
    )
    set(TGT_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${BASE_FOLDER})
    message(STATUS "Creating ${TGT_BASE_DIR}")
    foreach (DIRECTORY IN LISTS FOLDER_NAMES)
        set (COPYABLE_FOLDER "${BASE_FOLDER}/${DIRECTORY}")
        add_custom_command(TARGET ${TGT} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/${COPYABLE_FOLDER} $<TARGET_FILE_DIR:${TGT}>/${COPYABLE_FOLDER}
                OUTPUT ${COPYING_OUTPUT})
        set(TGT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${COPYABLE_FOLDER})
    endforeach ()

    add_custom_command(TARGET ${TGT} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini $<TARGET_FILE_DIR:${TGT}>/imgui.ini
            OUTPUT ${COPYING_OUTPUT})
endfunction()
