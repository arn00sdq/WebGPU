#ifndef WEBGPU_CODING_UTILITIES_LOADER_HPP_
#define WEBGPU_CODING_UTILITIES_LOADER_HPP_

#include "../common/DataTypes.hpp"
#include <filesystem>

namespace loader
{
    bool loadGeometry(std::filesystem::path const &path,
                      std::vector<float> &pointData,
                      std::vector<float> &colorData,
                      std::vector<uint16_t> &indexData);

    WGPUShaderModule loadShaderModule(std::filesystem::path const &path,
                                      WGPUDevice device);

}

#endif