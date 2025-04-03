#ifndef WEBGPU_CODINGUTILITIES_UTILITIES_H_
#define WEBGPU_CODINGUTILITIES_UTILITIES_H_

#include "webgpu/webgpu.hpp"
#ifdef WEBGPU_BACKEND_WGPU
#include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__

#include <iostream>
#include <cassert>
#include <vector>
#include <ostream>
#include <array>

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

    WGPURequiredLimits getRequiredLimits(WGPUAdapter adapter);

    WGPUTextureFormat initializeSurface(WGPUSurface surface,
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

    WGPUShaderModule createShaderModule(WGPUDevice device);

    WGPURenderPipeline createRenderPipeline(WGPUDevice device,
                                            WGPUShaderModule shaderModule,
                                            WGPUTextureFormat surfaceFormat);

}

#endif