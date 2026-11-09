#include "Layer.h"

std::ostream &operator<<(std::ostream &os, const CatRuntime::Layer &proxy) {
  std::string layerStr((std::string)proxy);
  os << layerStr;
  return os;
}

namespace CatRuntime {

Layer::operator std::string() const {
  if (m_EntityID == 0)
    return std::string();
  return Object_GetLayer(m_EntityID);
}

bool Layer::operator==(const std::string &other) const {
  if (m_EntityID == 0)
    return false;
  std::string thisStr = Object_GetLayer(m_EntityID);

  if (thisStr == other)
    return true;

  return false;
}

bool Layer::operator==(const char *other) const {
  if (m_EntityID == 0)
    return false;
  std::string thisStr = Object_GetLayer(m_EntityID);
  std::string otherStr(other);
  if (thisStr == otherStr)
    return true;

  return false;
}

Layer &Layer::operator=(const std::string &other) {
  if (m_EntityID == 0)
    return *this;
  Object_SetLayer(m_EntityID, other.c_str());
  return *this;
}

} // namespace CatRuntime
