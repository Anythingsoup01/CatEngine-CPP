#pragma once

#include "AssetManagerBase.h"
#include <cstdint>

namespace CatEngine {

using AssetRegistry = std::unordered_map<AssetHandle, Asset::MetaData>;
using AssetRegistryFileMap = std::unordered_map<uint32_t, AssetHandle>;

class EditorAssetManager : public AssetManagerBase {
public:
  ~EditorAssetManager();
  virtual Ref<Asset> GetAsset(AssetHandle handle) override;
  virtual bool IsAssetHandleValid(AssetHandle handle) const override;
  virtual bool IsAssetLoaded(AssetHandle handle) const override;
  virtual AssetType GetAssetType(AssetHandle handle) const override;
  virtual AssetHandle GetAssetHandle(const uint32_t &fileHash) const override;
  virtual Asset::MetaData GetAssetMetaData(const uint32_t &fileHash) const override;

  AssetHandle ImportAsset(const std::filesystem::path &filePath);
  void DeleteAsset(AssetHandle handle); // TODO: WORK ON THIS

  const Asset::MetaData &GetMetaData(AssetHandle handle) const;
  const std::filesystem::path &GetFilePath(AssetHandle handle) const;
  const std::filesystem::path &GetSourceFilePath(AssetHandle handle) const;

  const AssetRegistry &GetAssetRegistry() const { return m_AssetRegistry; }

  bool IsAssetExtension(const std::string &extension);

  void SerializeAssetRegistry();
  bool DeserializeAssetRegistry();

private:
  AssetRegistry m_AssetRegistry;
  AssetRegistryFileMap m_AssetRegistryFileMap;
  AssetMap m_LoadedAssets;
};
} // namespace CatEngine
