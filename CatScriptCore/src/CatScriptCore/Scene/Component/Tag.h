#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
namespace CatRuntime {
struct Tag {
public:
  uint64_t id;

  operator std::string() const {
    if (id == 0)
      return std::string();
    return Object_GetTag(id);
  }

  bool operator==(const std::string &other) const {
    if (id == 0)
      return false;
    std::string thisStr = Object_GetTag(id);

    if (thisStr == other)
      return true;

    return false;
  }

  bool operator==(const char *other) const {
    if (id == 0)
      return false;
    std::string thisStr = Object_GetTag(id);
    std::string otherStr(other);
    if (thisStr == otherStr)
        return true;

    return false;
  }

  Tag &operator=(const std::string &other) {
    if (id == 0)
      return *this;
    Object_SetTag(id, other.c_str());
    return *this;
  }

};
} // namespace CatRuntime

std::ostream &operator<<(std::ostream &os, const CatRuntime::Tag &proxy);
