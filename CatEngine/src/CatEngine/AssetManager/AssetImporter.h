#pragma once

#include "Asset.h"

namespace CatEngine
{
    class AssetImporter
    {
    public:
        static Ref<Asset> ImportAsset(const AssetHandle& handle, const Asset::MetaData& metaData);
    };
}
