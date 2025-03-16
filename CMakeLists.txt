cmake_minimum_required(VERSION 3.0...3.25)

add_executable(WebGPU main.cpp)

set_target_properties(WebGPU PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

add_subdirectory(webgpu)

target_link_libraries(WebGPU PRIVATE webgpu)

target_copy_webgpu_binaries(WebGPU)

set_target_properties(WebGPU PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
)
