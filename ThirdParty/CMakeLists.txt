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

function(add_entt)
    add_subdirectory(entt)
endfunction()

function(add_imguizmo)
    add_subdirectory(imguizmo)
endfunction()

function(add_json)
    add_subdirectory(json)
endfunction()

function(add_googletest)
    add_subdirectory(googletest)
endfunction()

function(add_magic_enum)
    add_subdirectory(magic_enum)
endfunction()

function(add_approval_tests)
    add_subdirectory(ApprovalTests)
endfunction()

function(add_fmt)
    add_subdirectory(fmt)
endfunction()

add_glfw()
add_glm()
add_vma()
add_imgui()
add_tinyobjloader()
add_stb_image()
add_thread_pool()
add_entt()
add_imguizmo()
add_json()
add_magic_enum()
add_fmt()

if (DISARRAY_BUILD_TESTS)
    add_approval_tests()
    add_googletest()
endif ()
