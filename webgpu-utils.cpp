#include "webgpu-utils.hpp"

WGPUAdapter webGPUUtils::requestAdapterSync(WGPUInstance instance,
                                            WGPURequestAdapterOptions const *options)
{
    struct UserData
    {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                    WGPUAdapter adapter,
                                    char const *message, void *pUserData)
    {
        UserData &userData = *reinterpret_cast<UserData *>(pUserData);
        if (status == WGPURequestAdapterStatus_Success)
        {
            userData.adapter = adapter;
        }
        else
        {
            std::cout << "Could not get WebGPU adapter: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    std::cout << "Requesting adapter..." << std::endl;

    wgpuInstanceRequestAdapter(
        instance /* equivalent of navigator.gpu */,
        options,
        onAdapterRequestEnded,
        (void *)&userData);

    assert(userData.requestEnded);

    return userData.adapter;
}

WGPUDevice webGPUUtils::requestDeviceSync(WGPUAdapter adapter,
                                          WGPUDeviceDescriptor const *descriptor)
{
    struct UserData
    {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status,
                                   WGPUDevice device,
                                   char const *message, void *pUserData)
    {
        UserData &userData = *reinterpret_cast<UserData *>(pUserData);
        if (status == WGPURequestDeviceStatus_Success)
        {
            userData.device = device;
        }
        else
        {
            std::cout << "Could not get WebGPU device: " << message << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(
        adapter,
        descriptor,
        onDeviceRequestEnded,
        (void *)&userData);

    assert(userData.requestEnded);

    return userData.device;
}

// We also add an inspect device function:
void webGPUUtils::inspectDevice(WGPUDevice device)
{
    std::vector<WGPUFeatureName> features;
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    features.resize(featureCount);
    wgpuDeviceEnumerateFeatures(device, features.data());

    std::cout << "Device features:" << std::endl;
    std::cout << std::hex;
    for (auto f : features)
    {
        std::cout << " - 0x" << f << std::endl;
    }
    std::cout << std::dec;

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
    bool success = wgpuDeviceGetLimits(device, &limits);
#endif

    if (success)
    {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << std::endl;
    }
}

WGPUInstance webGPUUtils::getInstance()
{
    WGPUInstanceDescriptor desc = {};
#ifdef WEBGPU_BACKEND_DAWN
    WGPUDawnTogglesDescriptor toggles;
    toggles.chain.next = nullptr;
    toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount = 0;
    toggles.enabledToggleCount = 1;
    const char *toggleName = "enable_immediate_error_handling";
    toggles.enabledToggles = &toggleName;

    desc.nextInChain = &toggles.chain;
#endif // WEBGPU_BACKEND_DAWN

    return wgpuCreateInstance(&desc);
}

WGPUAdapter webGPUUtils::getAdapter(WGPUInstance const &instance, WGPUSurface surface)
{
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    adapterOpts.compatibleSurface = surface;
    WGPUAdapter adapter = webGPUUtils::requestAdapterSync(instance, &adapterOpts);
    std::cout << "Got adapter: " << adapter << std::endl;
    return adapter;
}

WGPUDevice webGPUUtils::getDevice(WGPUAdapter adapter)
{
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = "My Device";
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason,
                                       char const *message, void * /* pUserData */)
    {
        std::cout << "Device lost: reason " << reason;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    WGPUDevice device = webGPUUtils::requestDeviceSync(adapter, &deviceDesc);
    std::cout << "Got device: " << device << std::endl;
    return device;
}

void webGPUUtils::initializeSurface(WGPUSurface surface,
                                    WGPUAdapter adapter, WGPUDevice device)
{
    /* size config */
    WGPUSurfaceConfiguration config = {};
    config.nextInChain = nullptr;
    config.width = 640;
    config.height = 480;

    /* format config (channels, size per channels, channels type  )*/
    WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
    config.format = surfaceFormat;
    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.device = device;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Auto;

    wgpuSurfaceConfigure(surface, &config);
}

WGPUCommandEncoder webGPUUtils::createEncoder(WGPUDevice device)
{
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "My command encoder";
    return wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
}
WGPURenderPassEncoder webGPUUtils::createRenderPass(WGPUCommandEncoder encoder, WGPUTextureView targetView)
{
    WGPURenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = targetView;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    return wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
}

WGPUCommandBuffer webGPUUtils::createCommandBuffer(WGPUCommandEncoder encoder)
{
    // Finally encode and submit the render pass
    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    // immuablle
    return wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
}
