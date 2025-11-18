#include "Layer.h"


std::ostream& operator<<(std::ostream& os, const CatRuntime::Layer::LayerProxy& proxy)
{
    std::string layerStr(CatRuntime::Object_GetLayer(proxy.id));
    os << layerStr;
    return os;
}
