cmake_minimum_required(VERSION 3.0...3.25)

if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    # Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_executable( WebGPU 
            main.cpp        
            application.hpp
            application.cpp
            webgpu-utils.hpp
            webgpu-utils.cpp
)

add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

target_link_libraries(WebGPU PRIVATE webgpu glfw glfw3webgpu)

target_copy_webgpu_binaries(WebGPU)

set_target_properties(WebGPU PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)
