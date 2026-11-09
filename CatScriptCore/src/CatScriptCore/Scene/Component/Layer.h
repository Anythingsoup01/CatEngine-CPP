#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include <cstdint>

namespace CatRuntime {
struct Layer {
protected:
  uint64_t m_EntityID;

public:
  Layer(const uint64_t &entityID) : m_EntityID(entityID) {}

  operator std::string() const;

  bool operator==(const std::string &other) const;

  bool operator==(const char *other) const;

  Layer &operator=(const std::string &other);
};
} // namespace CatRuntime

std::ostream &operator<<(std::ostream &os, const CatRuntime::Layer &proxy);
