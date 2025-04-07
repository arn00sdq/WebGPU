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
    size_t img_size = m_width * m_height * m_channels;
    for (auto p = data; p != data + img_size; p += m_channels)
    {
        m_r.push_back(static_cast<int>(p[0]));
        std::cout << "R: " << static_cast<int>(p[0]) << std::endl;
        m_g.push_back(static_cast<int>(p[1]));
        m_b.push_back(static_cast<int>(p[2]));
        if (m_channels == 4)
             m_blending.push_back(static_cast<int>(p[3]));
    }
    stbi_image_free(data);
}