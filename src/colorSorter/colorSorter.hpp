#ifndef WEBGPU_SRC_COLOR_SORTER_HPP_
#define WEBGPU_SRC_COLOR_SORTER_HPP_

#include "common/DataTypes.hpp"
#include <utility>   

class ImageData
{
public:
    struct PixelColor
    {
        int m_r;
        int m_g;
        int m_b;
        int m_blending;
    };

    ImageData(std::string const &filename);

    void sortColor();

    std::string m_filename;

    int m_height, m_width, m_channels;

    std::vector<PixelColor> m_pixels;

    std::unordered_map<unsigned long, int> m_sortedPixels; //rename
};

#endif