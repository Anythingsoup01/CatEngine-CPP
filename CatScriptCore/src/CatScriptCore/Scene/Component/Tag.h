#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include <cstdint>
namespace CatRuntime {
struct Tag {
public:
  Tag(const uint64_t &entityID) : m_EntityID(entityID) {}

  operator std::string() const;

  bool operator==(const std::string &other) const;

  bool operator==(const char *other) const;

  Tag &operator=(const std::string &other);

private:
  uint64_t m_EntityID;
};
} // namespace CatRuntime

std::ostream &operator<<(std::ostream &os, const CatRuntime::Tag &proxy);
