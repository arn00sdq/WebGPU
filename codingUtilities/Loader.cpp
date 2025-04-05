#include "../codingUtilities/Loader.hpp"
#include <fstream>
#include "Loader.hpp"

bool loader::loadGeometry(std::filesystem::path const &path,
                          std::vector<float> &pointData,
                          std::vector<float> &colorData,
                          std::vector<uint16_t> &indexData)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }

    pointData.clear();
    colorData.clear();
    indexData.clear();

    enum class Section
    {
        None,
        Points,
        Colors,
        Indices,
    };

    Section currentSection = Section::None;

    float value;
    uint16_t index;
    std::string line;
    while (!file.eof())
    {
        getline(file, line);

        // overcome the `CRLF` problem
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line == "[points]")
        {
            currentSection = Section::Points;
        }
        else if (line == "[indices]")
        {
            currentSection = Section::Indices;
        }
        else if (line == "[colors]")
        {
            currentSection = Section::Colors;
        }

        else if (line[0] == '#' || line.empty())
        {
            // Do nothing, this is a comment
        }
        else if (currentSection == Section::Points)
        {
            std::istringstream iss(line);
            // Get x,y
            for (int i = 0; i < 2; ++i)
            {
                iss >> value;
                pointData.push_back(value);
            }
        }
        else if (currentSection == Section::Colors)
        {
            std::istringstream iss(line);
            // Get r,g,b
            for (int i = 0; i < 3; ++i)
            {
                iss >> value;
                colorData.push_back(value);
            }
        }
        else if (currentSection == Section::Indices)
        {
            std::istringstream iss(line);
            // Get corners #0 #1 and #2
            for (int i = 0; i < 3; ++i)
            {
                iss >> index;
                indexData.push_back(index);
            }
        }
    }
    return true;
}

WGPUShaderModule loader::loadShaderModule(std::filesystem::path const &path,
                                          WGPUDevice device)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);

    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();

    WGPUShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = nullptr;
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    return wgpuDeviceCreateShaderModule(device, &shaderDesc);
}