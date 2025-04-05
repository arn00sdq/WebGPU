#include "application.hpp"
#include "webgpu-utils.hpp"
#include <cassert>

// We define a function that hides implementation-specific variants of device polling:
void wgpuPollEvents([[maybe_unused]] WGPUDevice device, [[maybe_unused]] bool yieldToWebBrowser)
{
#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(device, false, nullptr);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
    if (yieldToWebBrowser)
    {
        emscripten_sleep(100);
    }
#endif
}

bool Application::Initialize()
{
    std::cout << "0" << std::endl;
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    m_instance = wgpuCreateInstance(nullptr);

    // connect our GLFW window to WebGPU
    m_surface = glfwGetWGPUSurface(m_instance, m_window);

    m_adapter = webGPUUtils::getAdapter(m_instance, m_surface);
    m_device = webGPUUtils::getDevice(m_adapter);
    webGPUUtils::inspectDevice(m_device);

    // WebGPU device has a single queue, which is used to send both commands and data
    m_queue = wgpuDeviceGetQueue(m_device);

    // configure surface
    m_surfaceFormat = webGPUUtils::initializeSurface(m_surface, m_adapter, m_device);

    wgpuAdapterRelease(m_adapter);

    InitializePipeline();

    InitializeBuffer();

    return true;
}

void Application::InitializePipeline()
{
    WGPUShaderModule shaderModule = webGPUUtils::createShaderModule(m_device);
    if (shaderModule == nullptr)
    {
        std::cerr << "Error: shaderModule is null!" << std::endl;
    }

    m_renderPipeline = webGPUUtils::createRenderPipeline(m_device, shaderModule, m_surfaceFormat);
    if (m_renderPipeline == nullptr)
    {
        std::cerr << "Error: m_renderPipeline is null!" << std::endl;
    }

    wgpuShaderModuleRelease(shaderModule);
}

void Application::InitializeBuffer()
{

    std::vector<float> pointData = {
        -0.5, -0.5, // Point #0 (A)
        +0.5, -0.5, // Point #1
        +0.5, +0.5, // Point #2 (C)
        -0.5, +0.5, // Point #3
    };

    std::vector<uint16_t> indexData = {
        0, 1, 2, // Triangle #0 connects points #0, #1 and #2
        0, 2, 3  // Triangle #1 connects points #0, #2 and #3
    };
    indexData.resize((indexData.size() + 1) & ~1);

    std::vector<float> colorData = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 0.0};

    m_indexCount = static_cast<uint16_t>(indexData.size());

    // Create vertex buffer
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.nextInChain = nullptr;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    bufferDesc.mappedAtCreation = false;

    bufferDesc.label = "Point buffer";
    bufferDesc.size = pointData.size() * sizeof(float);
    bufferDesc.size = (bufferDesc.size + 3) & ~3;
    m_pointBuffer = wgpuDeviceCreateBuffer(m_device, &bufferDesc);
    wgpuQueueWriteBuffer(m_queue, m_pointBuffer, 0, pointData.data(), bufferDesc.size);

    bufferDesc.label = "color buffer";
    bufferDesc.size = colorData.size() * sizeof(float);
    bufferDesc.size = (bufferDesc.size + 3) & ~3;
    m_colorBuffer = wgpuDeviceCreateBuffer(m_device, &bufferDesc);
    wgpuQueueWriteBuffer(m_queue, m_colorBuffer, 0, colorData.data(), bufferDesc.size);

    bufferDesc.label = "Index buffer";
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    bufferDesc.size = indexData.size() * sizeof(uint16_t);
    bufferDesc.size = (bufferDesc.size + 3) & ~3;
    m_indexBuffer = wgpuDeviceCreateBuffer(m_device, &bufferDesc);
    wgpuQueueWriteBuffer(m_queue, m_indexBuffer, 0, indexData.data(), bufferDesc.size);
}

void Application::MainLoop()
{
    glfwPollEvents();

    // 1. Get the next target texture view
    WGPUTextureView targetView = GetNextSurfaceTextureView();
    if (!targetView)
        return;

    // 2. Draw things
    Draw(targetView);

    // 3. present the next texture of its swap chain
    wgpuSurfacePresent(m_surface);

    wgpuDeviceTick(m_device);
}

void Application::Draw(WGPUTextureView targetView)
{
    // 1. Create command encoder ( struct containing all command, render pass and compute pass )
    WGPUCommandEncoder encoder = webGPUUtils::createEncoder(m_device);

    // descriptor (color , depth ,etc)
    WGPURenderPassEncoder renderPass = webGPUUtils::createRenderPass(encoder, targetView);
    wgpuRenderPassEncoderSetPipeline(renderPass, m_renderPipeline);
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_pointBuffer, 0, wgpuBufferGetSize(m_pointBuffer));
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 1, m_colorBuffer, 0, wgpuBufferGetSize(m_colorBuffer));
    wgpuRenderPassEncoderSetIndexBuffer(renderPass, m_indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(m_indexBuffer));
    wgpuRenderPassEncoderDrawIndexed(renderPass, m_indexCount, 1, 0, 0, 0);

    // 2. Encode render pas
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    WGPUCommandBuffer command = webGPUUtils::createCommandBuffer(encoder);
    if (command == nullptr)
    {
        std::cerr << "Error: Command buffer is null!" << std::endl;
        return;
    }
    // release because we've created immuatable memory space so don't' need encoder anymore
    wgpuCommandEncoderRelease(encoder);

    std::cout << "Submitting command..." << std::endl;
    // 3. submit
    wgpuQueueSubmit(m_queue, 1, &command);
    wgpuCommandBufferRelease(command);
    std::cout << "Command submitted." << std::endl;

    wgpuTextureViewRelease(targetView);
}

WGPUTextureView Application::GetNextSurfaceTextureView()
{
    /* container for the multiple things that this function returns */
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success)
    {
        return nullptr;
    }

    WGPUTextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    // represent a sub-part of the texture
    // surfaceTexture.texture is the texture that we must draw on during this frame.
    WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);
    wgpuTextureRelease(surfaceTexture.texture);

    return targetView;
}

bool Application::IsRunning()
{
    return !glfwWindowShouldClose(m_window);
}

void Application::Terminate()
{
    wgpuBufferRelease(m_pointBuffer);
    wgpuBufferRelease(m_indexBuffer);
    wgpuBufferRelease(m_colorBuffer);

    wgpuRenderPipelineRelease(m_renderPipeline);
    glfwDestroyWindow(m_window);

    wgpuQueueRelease(m_queue);
    wgpuDeviceRelease(m_device);
    wgpuSurfaceRelease(m_surface);

    glfwTerminate();
}

////
//  TOOLS TO UNDERSTAND
////

void Application::messingWithBuffer()
{
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.nextInChain = nullptr;
    bufferDesc.label = "Some GPU-side data buffer";
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
    bufferDesc.size = 16;
    bufferDesc.mappedAtCreation = false;
    m_buffer1 = wgpuDeviceCreateBuffer(m_device, &bufferDesc);

    WGPUBufferDescriptor bufferDesc2 = {};
    bufferDesc2.nextInChain = nullptr;
    bufferDesc2.label = "Output buffer";
    bufferDesc2.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
    bufferDesc2.size = 16;
    bufferDesc2.mappedAtCreation = false;
    m_buffer2 = wgpuDeviceCreateBuffer(m_device, &bufferDesc2);

    std::vector<uint8_t> numbers(16);
    for (uint8_t i = 0; i < 16; ++i)
        numbers[i] = i;

    wgpuQueueWriteBuffer(m_queue, m_buffer1, 0, numbers.data(), numbers.size());

    WGPUCommandEncoder encoder = webGPUUtils::createEncoder(m_device);

    wgpuCommandEncoderCopyBufferToBuffer(encoder, m_buffer1, 0, m_buffer2, 0, 16);

    WGPUCommandBuffer command = webGPUUtils::createCommandBuffer(encoder);

    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit(m_queue, 1, &command);
    wgpuCommandBufferRelease(command);

    struct Context
    {
        bool ready;
        WGPUBuffer buffer;
    };

    auto onBuffer2Mapped = [](WGPUBufferMapAsyncStatus status, void *pUserData)
    {
        Context *context = reinterpret_cast<Context *>(pUserData);
        // We set ready to 'true'
        context->ready = true;
        if (status != WGPUBufferMapAsyncStatus_Success)
            return;

        std::cout << "Buffer 2 mapped with status " << status << std::endl;

        // Get a pointer to wherever the driver mapped the GPU memory to the RAM
        uint8_t *bufferData = (uint8_t *)wgpuBufferGetConstMappedRange(context->buffer, 0, 16);

        std::cout << "bufferData = [";
        for (int i = 0; i < 16; ++i)
        {
            if (i > 0)
                std::cout << ", ";
            std::cout << (int)bufferData[i];
        }
        std::cout << "]" << std::endl;
        wgpuBufferUnmap(context->buffer);
    };

    Context context = {false, m_buffer2};

    wgpuBufferMapAsync(m_buffer2, WGPUMapMode_Read, 0, 16, onBuffer2Mapped, (void *)&context);

    while (!context.ready)
    {
        //  ^^^^^^^^^^^^^ Use context.ready here instead of ready
        wgpuPollEvents(m_device, true);
    }
}

void Application::testCommandQueue()
{
    auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void * /* pUserData */)
    {
        std::cout << "Queued work finished with status: " << status << std::endl;
    };
    wgpuQueueOnSubmittedWorkDone(m_queue, onQueueWorkDone, nullptr /* pUserData */);

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);
    wgpuCommandEncoderInsertDebugMarker(encoder, "Test");

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    std::array<WGPUCommandBuffer, 3> commands;
    commands[0] = command;
    commands[1] = command;
    commands[2] = command;

    std::cout << "Submitting command..." << std::endl;
    wgpuQueueSubmit(m_queue, commands.size(), commands.data());
    wgpuCommandBufferRelease(command);
    std::cout << "Command submitted." << std::endl;

    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Tick/Poll device..." << std::endl;
        wgpuDeviceTick(m_device);
    }
}
