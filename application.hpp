#ifndef WEBGPU_APPLICATION_H_
#define WEBGPU_APPLICATION_H_

#include "webgpu-utils.hpp"

class Application
{
public:
    bool Initialize();

    void Terminate();

    void MainLoop();

    bool IsRunning();

    void testCommandQueue();

private:


    WGPUInstance m_instance;
    GLFWwindow *m_window;
    WGPUQueue m_queue;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;
};

#endif