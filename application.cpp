#include "application.hpp"

bool Application::Initialize()
{
    m_instance = webGPUUtils::getInstance();

    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    m_window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
    m_surface = glfwGetWGPUSurface(m_instance, m_window);

    if (!m_window)
    {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    m_adapter = webGPUUtils::getAdapter(m_instance, m_surface);
    m_device = webGPUUtils::getDevice(m_adapter);
    webGPUUtils::inspectDevice(m_device);

    m_queue = wgpuDeviceGetQueue(m_device);

    wgpuAdapterRelease(m_adapter);

    return true;
}

void Application::Terminate()
{
    glfwDestroyWindow(m_window);

    wgpuQueueRelease(m_queue);
    wgpuDeviceRelease(m_device);
    wgpuSurfaceRelease(m_surface);

    glfwTerminate();
}

void Application::MainLoop()
{
    glfwPollEvents();
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
