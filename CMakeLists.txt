add_executable(WebGPU main.cpp)

set_target_properties(WebGPU PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

add_subdirectory(WebGPU-distribution-dawn-6536)