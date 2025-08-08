#pragma once

namespace CatEngine
{
    class Utils
    {
    public:
        static std::string ReadFile(const std::filesystem::path& filePath);
    };
}
