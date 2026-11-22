#pragma once

#include "AssetManagerBase.h"
#include "Capybara/JIT/FileWatcher.h"
#include "CatEngine/AssetManager/Asset.h"
#include <cstdint>
#include <filesystem>

namespace CatEngine {

using AssetRegistry = std::unordered_map<AssetHandle, Asset::MetaData>;
using AssetRegistryFileMap = std::unordered_map<uint32_t, AssetHandle>;

class EditorAssetManager : public AssetManagerBase {
public:
  ~EditorAssetManager();
  virtual void Init() override;
  virtual void Poll() override;

  virtual Ref<Asset> GetAsset(AssetHandle handle) override;
  virtual bool IsAssetHandleValid(AssetHandle handle) const override;
  virtual bool IsAssetLoaded(AssetHandle handle) const override;
  virtual AssetType GetAssetType(AssetHandle handle) const override;
  virtual AssetHandle GetAssetHandle(const uint32_t &fileHash) const override;
  virtual Asset::MetaData GetAssetMetaData(const uint32_t &fileHash) const override;

  // When creating a new asset, this is called
  AssetHandle ImportNewAsset(const std::filesystem::path &filePath);

  // Used when loading an asset from disk
  AssetHandle ImportAssetFromDisk(const AssetHandle &handle, const Asset::MetaData &metaData);

  // Used when editing an assets metadata
  AssetHandle ReloadAssetWithNewMetaData(const AssetHandle &handle, const Asset::MetaData &metaData);

  // Used when the asset get's a file update to it's source data
  bool ReloadAssetFromDisk(const std::filesystem::path &filePath);

  AssetHandle GetAssetIDFromPath(const std::filesystem::path &filePath);

  void DeleteAsset(AssetHandle handle, bool deleteSource = true, bool retainSceneFocus = false);

  const Asset::MetaData &GetMetaData(AssetHandle handle) const;
  Asset::MetaData &GetMetaData(AssetHandle handle);

  const std::filesystem::path &GetFilePath(AssetHandle handle) const;
  const std::filesystem::path &GetSourceFilePath(AssetHandle handle) const;

  const AssetRegistry &GetAssetRegistry() const { return m_AssetRegistry; }

  bool IsAssetExtension(const std::string &extension);

  void SerializeAsset(const AssetHandle &handle);

  bool DeserializeAssetRegistry();
  void RebuildAssetRegistry();

private:
  AssetRegistry m_AssetRegistry;
  AssetRegistryFileMap m_AssetRegistryFileMap;
  AssetMap m_LoadedAssets;

  Capybara::FileWatcher m_FileWatcher;
};
} // namespace CatEngine
