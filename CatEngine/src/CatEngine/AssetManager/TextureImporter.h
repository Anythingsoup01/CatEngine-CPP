#pragma once

#include "Asset.h"

#include "CatEngine/Renderer/Texture.h"

namespace CatEngine
{
    class TextureImporter
    {
    public:
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, Ref<Asset::MetaData>& metaData);
        static Ref<Texture2D> ConstImportTexture2D(AssetHandle handle, const Ref<Asset::MetaData>& metaData);
        static Ref<Texture2D> ImportIconTexture(const TextureSpecification& spec, Ref<Asset::MetaData>& metaData);
    };
}
