#include "Tag.h"

std::ostream& operator<<(std::ostream& os, const CatRuntime::Tag& proxy)
{
    std::string tagStr(Object_GetTag(proxy.id));
    os << tagStr;
    return os;
}
