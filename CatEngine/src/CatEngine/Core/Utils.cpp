#include "cepch.h"
#include "Utils.h"

namespace CatEngine
{
    std::string Utils::ReadFile(const std::filesystem::path &filePath)
    {
        std::ifstream in(filePath, std::ios::binary);
        if (in)
        {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return contents;
        }
        CE_API_ERROR("Failed to open file at {0}", filePath.string().c_str());
        return "";
    }
}
