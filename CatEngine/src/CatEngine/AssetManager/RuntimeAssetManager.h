#pragma once

#include "AssetManagerBase.h"

namespace CatEngine
{
    class RuntimeAssetManager : public AssetManagerBase
    {
    public:
        virtual Ref<Asset> GetAsset(AssetHandle handle);


    };
}
