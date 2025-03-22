#ifndef WEBGPU_CODINGUTILITIES_UTILITIES_H_
#define WEBGPU_CODINGUTILITIES_UTILITIES_H_

#include <iostream>
#include <vector>
#include <array>
#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>

#include <cassert>

WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const *options);

WGPUDevice requestDeviceSync(WGPUAdapter adapter, WGPUDeviceDescriptor const *descriptor);

void inspectDevice(WGPUDevice device);

WGPUInstance getInstance();

WGPUAdapter getAdapter(WGPUInstance const &instance);

WGPUDevice getDevice(WGPUAdapter adapter);

#endif