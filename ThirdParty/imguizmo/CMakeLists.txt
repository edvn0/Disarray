cmake_minimum_required(VERSION 3.22)

project(imguizmo CXX)

add_library(${PROJECT_NAME} STATIC include/GraphEditor.h include/ImCurveEdit.h include/ImGradient.h include/ImGuizmo.h include/ImSequencer.h include/ImZoomSlider.h src/ImCurveEdit.cpp src/ImGradient.cpp src/ImGuizmo.cpp src/ImSequencer.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include PRIVATE ${CMAKE_SOURCE_DIR}/ThirdParty/imgui)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
