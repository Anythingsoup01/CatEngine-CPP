#pragma once

#include "CatEngine/Core/UUID.h"

#include "Asset.h"

namespace CatEngine
{
    using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;
    class AssetManagerBase
    {
    public:
        virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
        virtual AssetType GetAssetType(AssetHandle handle) const = 0;
    };
}
