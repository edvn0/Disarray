function(add_glfw)
    add_subdirectory(glfw)
endfunction(add_glfw)

function(add_glm)
    add_subdirectory(glm)
endfunction()

function(add_vma)
    add_subdirectory(VulkanMemoryAllocator)
endfunction()

function(add_imgui)
    add_subdirectory(imgui)
endfunction()

function(add_tinyobjloader)
    add_subdirectory(tinyobjloader)
endfunction()

function(add_stb_image)
    add_subdirectory(stb_image)
endfunction()

function(add_thread_pool)
    add_subdirectory(thread-pool)
endfunction()

add_glfw()
add_glm()
add_vma()
add_imgui()
add_tinyobjloader()
add_stb_image()
add_thread_pool()
