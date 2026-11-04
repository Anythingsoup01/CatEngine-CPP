#include "Layer.h"


std::ostream& operator<<(std::ostream& os, const CatRuntime::Layer& proxy)
{
    std::string layerStr(Object_GetLayer(proxy.id));
    os << layerStr;
    return os;
}
