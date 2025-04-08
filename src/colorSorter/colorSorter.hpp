#ifndef WEBGPU_SRC_COLOR_SORTER_HPP_
#define WEBGPU_SRC_COLOR_SORTER_HPP_

#include "common/DataTypes.hpp"

class ImageData
{
public:
    ImageData(std::string const &filename);

    sortColor();

    std::string m_filename;

    int m_height, m_width, m_channels;

    std::vector<int> m_r;
    std::vector<int> m_g;
    std::vector<int> m_b;
    std::vector<int> m_blending;
    
    std::vector<int> m_sortedPixel;

};

#endif