#ifndef WEBGPU_APPLICATION_H_
#define WEBGPU_APPLICATION_H_

#include "webgpu-utils.hpp"

class Application
{
public:
    bool Initialize();

    void MainLoop();

    void Draw(WGPUTextureView targetView);

    void Terminate();

    bool IsRunning();

    void testCommandQueue();

private:
    /**
     * @brief Ask each frame the next available texture
     * @return A valid view
     */
    WGPUTextureView GetNextSurfaceTextureView();

    WGPUInstance m_instance;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;

    WGPUSurface m_surface;
    WGPUQueue m_queue;

    GLFWwindow *m_window;
};

#endif