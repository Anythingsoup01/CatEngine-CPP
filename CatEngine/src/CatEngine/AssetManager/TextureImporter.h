#pragma once

#include "Asset.h"

#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
    class TextureImporter
    {
    public:
        // This version let's us import a texture and modify the meta data to properly fit the texture
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, Asset::MetaData& metaData);

        // This version is used by the EditorAssetManager when loading meta data from a file
        static Ref<Texture2D> ConstImportTexture2D(AssetHandle handle, const Asset::MetaData& metaData);

        // This version is used in conjunction with both of the importers
        static Ref<Texture2D> ImportIconTexture(const TextureSpecification& spec, Asset::MetaData& metaData);
        
        // This version is used for icons
        static Ref<Texture2D> ImportIconTexture(const std::filesystem::path& filePath);
    };
}
