#pragma once

#include "AssetManagerBase.h"

namespace CatEngine
{

    using AssetRegistry = std::unordered_map<AssetHandle, Asset::MetaData>;

    class EditorAssetManager : public AssetManagerBase
    {
    public:
        ~EditorAssetManager();
        virtual Ref<Asset> GetAsset(AssetHandle handle) override;
        virtual bool IsAssetHandleValid(AssetHandle handle) const override;
        virtual bool IsAssetLoaded(AssetHandle handle) const override;
        virtual AssetType GetAssetType(AssetHandle handle) const override;

        AssetHandle ImportAsset(const std::filesystem::path& filePath);
        void DeleteAsset(AssetHandle handle); // TODO: WORK ON THIS

        const Asset::MetaData& GetMetaData(AssetHandle handle) const;
        const std::filesystem::path& GetFilePath(AssetHandle handle) const;

        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

        void SerializeAssetRegistry();
        bool DeserializeAssetRegistry();
    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
    };
}
