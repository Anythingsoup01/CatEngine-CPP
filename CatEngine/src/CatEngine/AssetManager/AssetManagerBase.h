#pragma once

#include "Asset.h"

namespace CatEngine {
using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;
class AssetManagerBase {
public:
  virtual void Init() = 0;
  virtual void Poll() = 0;
  virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
  virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
  virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
  virtual AssetType GetAssetType(AssetHandle handle) const = 0;
  virtual AssetHandle GetAssetHandle(const uint32_t &fileHash) const = 0;
  virtual Asset::MetaData GetAssetMetaData(const uint32_t &fileHash) const = 0;
};
} // namespace CatEngine
