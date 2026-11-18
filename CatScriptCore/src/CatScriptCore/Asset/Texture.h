#pragma once

#include <cstdint>
namespace CatRuntime {

struct Texture {
protected:
  uint64_t m_AssetID = 0;
public:
  Texture() = default;
  Texture(const uint64_t &assetID)
    : m_AssetID(assetID) {}

  const uint64_t &GetID() const { return m_AssetID; }
};

}
