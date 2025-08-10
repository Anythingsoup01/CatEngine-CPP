#include "cepch.h"
#include "OpenGLTexture2D.h"

#include "stb_image.h"

namespace CatEngine
{
    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& filePath)
        : m_Path(filePath)
    {
        int width, height, channels;
        stbi_uc* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    }
}
