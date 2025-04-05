cmake_minimum_required(VERSION 3.0...3.25)

if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    # Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_executable(  WebGPU
        main.cpp        
        application.hpp
        application.cpp
        webgpu-utils.hpp
        webgpu-utils.cpp
        codingUtilities/Loader.cpp
)

target_compile_definitions(WebGPU PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


target_include_directories(WebGPU PRIVATE
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/codingUtilities
)

add_subdirectory(webgpu)
add_subdirectory(glfw3webgpu)

target_link_libraries(WebGPU PRIVATE glfw webgpu  glfw3webgpu)



target_copy_webgpu_binaries(WebGPU)

set_target_properties(WebGPU PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

if (MSVC)
	target_compile_options(WebGPU PRIVATE /W4)
else()
	target_compile_options(WebGPU PRIVATE -Wall -Wextra -pedantic)
endif()

if (XCODE)
	set_target_properties(App PROPERTIES
		XCODE_GENERATE_SCHEME ON
		XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
	)
endif()