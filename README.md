Library you need to install : 

GLFW : simplify window managment
https://eliemichel.github.io/LearnWebGPU/getting-started/opening-a-window.html 

GLFW3WGPU :  connect GLFW window to WebGPU
https://github.com/eliemichel/glfw3webgpu/releases/download/v1.2.0/glfw3webgpu-v1.2.0.zip

# Build with only X11 support
cmake -B build -DGLFW_BUILD_WAYLAND=OFF