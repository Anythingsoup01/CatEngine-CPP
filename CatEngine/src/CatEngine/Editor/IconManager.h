#pragma once

#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
    class IconManager
    {
    public:
        static void Init();
        static void Shutdown();

        static Ref<Texture2D> AddIcon(const std::filesystem::path& filePath, const std::string& nameOverload = "");
        static Ref<Texture2D> GetIcon(const char* iconName);
    private:
        static inline std::unordered_map<uint32_t, Ref<Texture2D>> s_IconMap;
    };
} 
