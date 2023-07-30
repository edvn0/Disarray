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

add_glfw()
add_glm()
add_vma()
add_imgui()
