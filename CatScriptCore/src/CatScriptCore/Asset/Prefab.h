#pragma once

#include <cstdint>

namespace CatRuntime {

struct Prefab {
protected:
  uint64_t m_AssetID = 0;
public:
  Prefab() = default;
  Prefab(const uint64_t &assetID) : m_AssetID(assetID) {}
  const uint64_t &GetAssetID() const { return m_AssetID; }

  operator bool() const { return m_AssetID != 0; }
};

}
