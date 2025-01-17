cmake_minimum_required(VERSION 3.5.0)
project(genxml-editor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(thirdparty/tinyxml2)
include_directories(thirdparty/imgui)

set(IMGUI_SRC
thirdparty/imgui/imgui.cpp
thirdparty/imgui/imgui_widgets.cpp
thirdparty/imgui/imgui_tables.cpp
thirdparty/imgui/imgui_draw.cpp)

set(IMGUI_IMPL_SRC
thirdparty/imgui/backends/imgui_impl_glfw.cpp
thirdparty/imgui/backends/imgui_impl_opengl3.cpp
thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
)

add_library(imguideps STATIC ${IMGUI_SRC})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)
find_package(glfw3)

add_executable(imgui-demo
    ${IMGUI_IMPL_SRC}
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/examples/example_glfw_opengl3/main.cpp)
target_link_libraries(imgui-demo PRIVATE imguideps glfw dl OpenGL)
target_include_directories(imgui-demo PUBLIC thirdparty/imgui/backends)

set(GENXML_EDITOR_SRC
    xml_parser.cpp
    xml_saver.cpp
    main_ui.cpp)

add_executable(genxml-editor
    main.cpp
    ${GENXML_EDITOR_SRC}
    ${IMGUI_IMPL_SRC}
)
target_link_libraries(genxml-editor PRIVATE tinyxml2 imguideps glfw OpenGL)
if (CMAKE_SYSTEM_NAME  STREQUAL "Linux")
target_link_libraries(genxml-editor PRIVATE dl pthread)
endif()
target_include_directories(genxml-editor PUBLIC
thirdparty/imgui/backends
thirdparty/imgui/misc/cpp)
