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

    m_r.reserve(m_width * m_height);
    m_g.reserve(m_width * m_height);
    m_b.reserve(m_width * m_height);
    m_sortedPixel.reserve(m_width * m_height);
    if (m_channels == 4)
        m_blending.reserve(m_width * m_height);
        
    for (auto p = data; p != data + img_size; p += m_channels)
    {
        m_r.push_back(static_cast<int>(p[0]));
        // std::cout << "R: " << static_cast<int>(p[0]) << std::endl;
        m_g.push_back(static_cast<int>(p[1]));
        m_b.push_back(static_cast<int>(p[2]));
        if (m_channels == 4)
            m_blending.push_back(static_cast<int>(p[3]));
    }

    stbi_image_free(data);
}

ImageData::sortColor()
{
    //todo
}
