#pragma once

#include "Asset.h"

#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
    class TextureImporter
    {
    public:
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const Asset::MetaData& metaData);
        static Ref<Texture2D> ImportIconTexture(const std::filesystem::path& filePath);
    };
}
