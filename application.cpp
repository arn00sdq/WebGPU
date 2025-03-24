#include "application.hpp"

bool Application::Initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    m_window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    m_instance = webGPUUtils::getInstance();
    
    // connect our GLFW window to WebGPU
    m_surface = glfwGetWGPUSurface(m_instance, m_window);

    m_adapter = webGPUUtils::getAdapter(m_instance, m_surface);
    m_device = webGPUUtils::getDevice(m_adapter);
    webGPUUtils::inspectDevice(m_device);
    
    // WebGPU device has a single queue, which is used to send both commands and data
    m_queue = wgpuDeviceGetQueue(m_device);
    
    // configure surface
    webGPUUtils::initializeSurface(m_surface, m_adapter, m_device);

    wgpuAdapterRelease(m_adapter);

    return true;
}

void Application::MainLoop()
{
    glfwPollEvents();

    //1. Get the next target texture view
    WGPUTextureView targetView = GetNextSurfaceTextureView();
    if (!targetView)
        return;

    //2. Draw things
    Draw(targetView);

    //3. present the next texture of its swap chain
    wgpuSurfacePresent(m_surface);

    wgpuDeviceTick(m_device);
}

void Application::Draw(WGPUTextureView targetView)
{
    // 1. Create command encoder 
    WGPUCommandEncoder encoder = webGPUUtils::createEncoder(m_device);
    WGPURenderPassEncoder renderPass = webGPUUtils::createRenderPass(encoder, targetView);

    //2. Encode render pas
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    WGPUCommandBuffer command = webGPUUtils::createCommandBuffer(encoder);
    wgpuCommandEncoderRelease(encoder);

    std::cout << "Submitting command..." << std::endl;
    //3. submit
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

void Application::Terminate()
{
    glfwDestroyWindow(m_window);

    wgpuQueueRelease(m_queue);
    wgpuDeviceRelease(m_device);
    wgpuSurfaceRelease(m_surface);

    glfwTerminate();
}

bool Application::IsRunning()
{
    return !glfwWindowShouldClose(m_window);
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
