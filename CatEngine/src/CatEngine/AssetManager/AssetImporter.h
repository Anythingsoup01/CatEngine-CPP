#pragma once

#include "Asset.h"

namespace CatEngine
{
    class AssetImporter
    {
    public:
        static Ref<Asset> ImportAsset(const AssetHandle& handle, Ref<Asset::MetaData>& metaData);
        static Ref<Asset> ImportAsset(const AssetHandle& handle, const Ref<Asset::MetaData>& metaData);
    };
}
