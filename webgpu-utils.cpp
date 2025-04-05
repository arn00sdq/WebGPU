#define WEBGPU_CPP_IMPLEMENTATION
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
    WGPURequiredLimits requiredLimits = getRequiredLimits(adapter);

    deviceDesc.requiredLimits = &requiredLimits;

    WGPUDevice device = webGPUUtils::requestDeviceSync(adapter, &deviceDesc);
    std::cout << "Got device: " << device << std::endl;

    auto onDeviceError = [](WGPUErrorType type, char const *message, void * /* pUserData */)
    {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

    /* some extra infos*/
    WGPUSupportedLimits supportedLimits;
    supportedLimits.nextInChain = nullptr;
    wgpuAdapterGetLimits(adapter, &supportedLimits);
    std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;
    std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;
    std::cout << "adapter.maxVertexBuffers: " << supportedLimits.limits.maxVertexBuffers << std::endl;

    wgpuDeviceGetLimits(device, &supportedLimits);
    std::cout << "device.maxVertexAttributes: " << requiredLimits.limits.maxVertexAttributes << std::endl;
    std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;
    std::cout << "device.maxVertexBuffers: " << supportedLimits.limits.maxVertexBuffers << std::endl;

    return device;
}

void setDefault(WGPULimits &limits)
{
    limits.maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
    limits.maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
    limits.minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
    limits.minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
    limits.maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
    limits.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
}

WGPURequiredLimits webGPUUtils::getRequiredLimits(WGPUAdapter adapter)
{
    WGPUSupportedLimits supportedLimits;
    supportedLimits.nextInChain = nullptr;
    wgpuAdapterGetLimits(adapter, &supportedLimits);

    WGPURequiredLimits requiredLimits{};
    setDefault(requiredLimits.limits);

    requiredLimits.limits.maxVertexAttributes = 2;
    // We should also tell that we use 1 vertex buffers
    requiredLimits.limits.maxVertexBuffers = 2;
    // Maximum size of a buffer is 6 vertices of 3 float each
    requiredLimits.limits.maxBufferSize = 6 * 3 * sizeof(float);
    // Maximum stride between 2 consecutive vertices in the vertex buffer
    requiredLimits.limits.maxVertexBufferArrayStride = 3 * sizeof(float);

    requiredLimits.limits.maxInterStageShaderComponents = 3;

    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

    return requiredLimits;
}
WGPUTextureFormat webGPUUtils::initializeSurface(WGPUSurface surface,
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

    return surfaceFormat;
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
    renderPassColorAttachment.clearValue = WGPUColor{1.0, 0.0, 0.0, 1.0};
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

WGPUShaderModule webGPUUtils::createShaderModule(WGPUDevice device)
{

    const char *shaderSource = R"(
      
        struct VertexInput
        {
            @location(0) position : vec2f,
            @location(1) color : vec3f,
        };

        struct VertexOutput 
        {
            @builtin(position) position: vec4f,
            @location(0) color: vec3f,
        };      

        @vertex
        fn vs_main(in: VertexInput) -> VertexOutput {
            let ratio = 640.0 / 480.0;
            var out : VertexOutput;
            out.position = vec4(in.position.x, in.position.y * ratio , 0.0,1.0);
            out.color = in.color;
            return out;
        }

        @fragment
        fn fs_main(in: VertexOutput) -> @location(0) vec4f {
            return vec4( in.color, 1.0 );
        }
        )";

    // Load the shader module
    WGPUShaderModuleDescriptor shaderDesc{};

    // We use the extension mechanism to specify the WGSL part of the shader module descriptor
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    // Set the chained struct's header
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    // Connect the chain
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderCodeDesc.code = shaderSource;
    return wgpuDeviceCreateShaderModule(device, &shaderDesc);
}

WGPURenderPipeline webGPUUtils::createRenderPipeline(WGPUDevice device,
                                                     WGPUShaderModule shaderModule,
                                                     WGPUTextureFormat surfaceFormat)
{
    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.nextInChain = nullptr;

    // ---- Configure vertex  ----
    // We do not use any vertex buffer for this first simplistic example
    pipelineDesc.vertex.bufferCount = 0;
    pipelineDesc.vertex.buffers = nullptr;

    // code
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    // ---- Configure Primitive  ----

    // primitive = primitive(a point, a line or a triangle) assembly and rasterization stages.
    //  Each sequence of 3 vertices is considered as a triangle
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    // When not specified, vertices are considered sequentially.
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

    // The face orientation is defined by assuming that when looking
    // from the front of the face, its corner vertices are enumerated in the counter-clockwise (CCW) order.
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;

    // Not hidding the faces pointing away from us (which is often used for optimization).
    // None when developing.
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;

    // ---- Configure Fragment  ----
    WGPUFragmentState fragmentState{};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    WGPUBlendState blendState{};
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.alpha.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_One;
    blendState.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;

    WGPUColorTargetState colorTarget{};
    colorTarget.format = surfaceFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some of the color channels.

    // We have only one target because our render pass has only one output color
    // attachment.
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    pipelineDesc.fragment = &fragmentState;
    if (pipelineDesc.fragment == nullptr)
    {
        std::cerr << "Error: Fragment state is null!" << std::endl;
    }

    // We do not use stencil/depth testing for now
    pipelineDesc.depthStencil = nullptr;

    // Samples per pixel
    pipelineDesc.multisample.count = 1;
    // Default value for the mask, meaning "all bits on"
    pipelineDesc.multisample.mask = ~0u;
    // Default value as well (irrelevant for count = 1 anyways)
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    pipelineDesc.layout = nullptr;

    // ----- Vertex Buffer Layout -----

    // Vertex fetch
    std::vector<WGPUVertexBufferLayout> vertexBuffersLayout(2);

    WGPUVertexAttribute positionAttribs;
    positionAttribs.shaderLocation = 0; // @location(0)
    positionAttribs.format = WGPUVertexFormat_Float32x2;
    positionAttribs.offset = 0;

    vertexBuffersLayout[0].attributeCount = 1;
    vertexBuffersLayout[0].attributes = &positionAttribs;
    vertexBuffersLayout[0].arrayStride = 2 * sizeof(float);
    vertexBuffersLayout[0].stepMode = WGPUVertexStepMode_Vertex;

    WGPUVertexAttribute colorAttribs;
    colorAttribs.shaderLocation = 1;                  // @location(1)
    colorAttribs.format = WGPUVertexFormat_Float32x3; // different type! rgb
    colorAttribs.offset = 0;

    vertexBuffersLayout[1].attributeCount = 1;
    vertexBuffersLayout[1].attributes = &colorAttribs;
    vertexBuffersLayout[1].arrayStride = 3 * sizeof(float);
    vertexBuffersLayout[1].stepMode = WGPUVertexStepMode_Vertex;

    pipelineDesc.vertex.bufferCount = 2;
    pipelineDesc.vertex.buffers = vertexBuffersLayout.data();

    return wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
}