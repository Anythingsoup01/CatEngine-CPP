#include "Tag.h"

std::ostream& operator<<(std::ostream& os, const CatRuntime::Tag& proxy)
{
    std::string tagStr((std::string)proxy);
    os << tagStr;
    return os;
}
CatRuntime::Tag::operator std::string() const {
  if (m_EntityID == 0)
    return std::string();
  return Object_GetTag(m_EntityID);
}
bool CatRuntime::Tag::operator==(const std::string &other) const {
  if (m_EntityID == 0)
    return false;
  std::string thisStr = Object_GetTag(m_EntityID);

  if (thisStr == other)
    return true;

  return false;
}
bool CatRuntime::Tag::operator==(const char *other) const {
  if (m_EntityID == 0)
    return false;
  std::string thisStr = Object_GetTag(m_EntityID);
  std::string otherStr(other);
  if (thisStr == otherStr)
    return true;

  return false;
}
CatRuntime::Tag &CatRuntime::Tag::operator=(const std::string &other) {
  if (m_EntityID == 0)
    return *this;
  Object_SetTag(m_EntityID, other.c_str());
  return *this;
}
