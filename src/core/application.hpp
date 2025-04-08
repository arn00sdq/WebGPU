#ifndef WEBGPU_APPLICATION_HPP_
#define WEBGPU_APPLICATION_HPP_

#include "codingUtilities/webgpu-utils.hpp"
#include "colorSorter/colorSorter.hpp"
#include <filesystem>

class Application
{
public:
    std::unique_ptr<ImageData> m_imageData;

    bool Initialize();

    void MainLoop();

    void Draw(WGPUTextureView targetView);

    void Terminate();

    bool IsRunning();

    void testCommandQueue();

    static constexpr std::string_view m_imgSortPath = "/resources/testSort.png";

private:
    /**
     * @brief Ask each frame the next available texture
     * @return A valid view
     */
    WGPUTextureView
    GetNextSurfaceTextureView();

    void InitializePipeline();

    void messingWithBuffer();

    void InitializeBuffer();

    WGPUInstance m_instance;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;

    WGPUSurface m_surface;
    WGPUQueue m_queue;

    WGPUTextureFormat m_surfaceFormat = WGPUTextureFormat_Undefined;
    WGPURenderPipeline m_renderPipeline;

    WGPUBuffer m_buffer1;
    WGPUBuffer m_buffer2;
    WGPUBuffer m_pointBuffer;
    WGPUBuffer m_indexBuffer;
    uint32_t m_indexCount;
    WGPUBuffer m_colorBuffer;

    GLFWwindow *m_window;
};

#endif