function(copy_directory_to_binary FOLDER_NAME DEPEND_TARGET)
	set(TARGET_NAME COPY_FOLDER_${FOLDER_NAME})
	add_custom_target(${TARGET_NAME}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${FOLDER_NAME})
	add_dependencies(${DEPEND_TARGET} ${TARGET_NAME})
endfunction()
