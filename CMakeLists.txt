cmake_minimum_required(VERSION 3.5.0)
project(genxml-editor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(thirdparty/imgui)

set(IMGUI_SRC
thirdparty/imgui/imgui.cpp
thirdparty/imgui/imgui_widgets.cpp
thirdparty/imgui/imgui_tables.cpp
thirdparty/imgui/imgui_draw.cpp)

add_library(imguideps STATIC ${IMGUI_SRC})

find_package(OpenGL)

add_executable(imgui-demo
    /data/sdb/genxml-editor/thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    thirdparty/imgui/examples/example_glfw_opengl3/main.cpp)
target_link_libraries(imgui-demo PRIVATE imguideps glfw dl ${OPENGL_gl_LIBRARY})
target_include_directories(imgui-demo PUBLIC thirdparty/imgui/backends)

add_executable(genxml-editor main.cpp)