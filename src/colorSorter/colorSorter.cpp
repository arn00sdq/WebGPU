#include "colorSorter/colorSorter.hpp"
#include "colorSorter.hpp"

ImageData::ImageData(std::string const &filename) : m_filename(filename)
{
    std::cout << m_filename << std::endl;
    unsigned char *data = stbi_load(filename.c_str(), &m_width, &m_height, &m_channels, 0);

    if (data == NULL)
    {
        std::cerr << "Could not load image! " << filename << std::endl;
        exit(1);
    }
    std::cout << "load image with width : " << m_width << " and heigh : " << m_height << std::endl;

    size_t img_size = m_width * m_height * m_channels;

    m_pixels.reserve(m_width * m_height);
    m_sortedPixels.reserve(m_width * m_height);

    for (auto p = data; p != data + img_size; p += m_channels)
    {
        PixelColor pixelColor{static_cast<int>(p[0]),
                              static_cast<int>(p[1]),
                              static_cast<int>(p[2]),
                              m_channels == 4 ? static_cast<int>(p[3]) : 255};
        m_pixels.push_back(pixelColor);
    }

    std::cout << m_pixels.size() << "-- " << std::endl;
    stbi_image_free(data);
}

void ImageData::sortColor()
{
    auto rbgToHex = [](int r, int g, int b)
    {
        return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
    };

    for (size_t i= 0; i < m_pixels.size(); ++i)
    {
        int hexValue = rbgToHex(m_pixels[i].m_r, m_pixels[i].m_g, m_pixels[i].m_b);
        auto sortedPixelItr = m_sortedPixels.find(hexValue);
        if (sortedPixelItr == nullptr)
        {
            m_sortedPixels.insert({hexValue, 1});
        }
        else
        {
            sortedPixelItr->second++;
        }
    }

    // for (auto& p : m_sortedPixels)
    //     std::cout << ' ' << p.first << " => " << p.second << '\n';
}
