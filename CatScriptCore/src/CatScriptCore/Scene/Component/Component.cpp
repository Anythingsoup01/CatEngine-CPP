#pragma once
#include "Component.h"

namespace CatRuntime
{
    Tag Component::Tag()
    {
        return GetComponent<Tag>();
    }

    Layer Component::Layer()
    {
        return GetComponent<Layer>();
    }

    Transform Component::Transform()
    {
        return GetComponent<Transform>();
    }
}
