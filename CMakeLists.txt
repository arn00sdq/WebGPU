cmake_minimum_required(VERSION 3.0...3.25)

if (NOT EMSCRIPTEN)
    add_subdirectory(external/glfw)
else()
    # Emscripten has built-in support for GLFW but requires the `-sUSE_GLFW=3` link option:
    add_library(external/glfw INTERFACE)
    target_link_options(external/glfw INTERFACE -sUSE_GLFW=3)
endif()

# === WebGPU Submodules ===
add_subdirectory(external/webgpu)
add_subdirectory(external/glfw3webgpu)

# === Executable ===
add_executable( WebGPU
        src/main/main.cpp        
        src/core/application.cpp
        src/codingUtilities/webgpu-utils.cpp
        src/codingUtilities/Loader.cpp
        src/common/ThirdPartyLibs.cpp
        src/colorSorter/colorSorter.cpp


)

target_compile_definitions(WebGPU PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# === Include Paths ===
target_include_directories(WebGPU PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw3webgpu
    ${CMAKE_CURRENT_SOURCE_DIR}/external/webgpu
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tools
)

# === Linking ===
target_link_libraries(WebGPU PRIVATE glfw webgpu glfw3webgpu)

# === WebGPU binaries ===
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