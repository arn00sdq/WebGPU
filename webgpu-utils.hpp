#ifndef WEBGPU_CODINGUTILITIES_UTILITIES_H_
#define WEBGPU_CODINGUTILITIES_UTILITIES_H_

#include <iostream>
#include <vector>
#include <array>
#include <webgpu/webgpu.h>
#include "glfw3webgpu/glfw3webgpu.h"
#include <GLFW/glfw3.h>

#include <cassert>

namespace webGPUUtils
{
    WGPUAdapter requestAdapterSync(WGPUInstance instance,
                                   WGPURequestAdapterOptions const *options);

    WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                                 WGPUDeviceDescriptor const *descriptor);

    void inspectDevice(WGPUDevice device);

    WGPUInstance getInstance();

    WGPUAdapter getAdapter(WGPUInstance const &instance, WGPUSurface surface);

    WGPUDevice getDevice(WGPUAdapter adapter);

    void initializeSurface(WGPUSurface surface,
                           WGPUAdapter adapter, WGPUDevice device);

    /**
     * @param device 
     * @return A command encoder for the draw call
     */
    WGPUCommandEncoder createEncoder(WGPUDevice device);
    
    /**
     * @brief Create render pass that clears the screen with our color
     * @param encoder Encoder for the draw call
     * @param targetView Target texture view
     * @return A command encoder for the draw call
     */
    WGPURenderPassEncoder createRenderPass(WGPUCommandEncoder encoder, WGPUTextureView targetView);

    /**
     * @note Content timeline GPU. Queue timeline GPU
     */
    WGPUCommandBuffer createCommandBuffer(WGPUCommandEncoder encoder);
}

#endif