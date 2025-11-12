#pragma once
#include "Component.h"

namespace CatRuntime
{
    TagComponent Component::Tag()
    {
        //return GetComponent<Tag>();
        return TagComponent();
    }

    LayerComponent Component::Layer()
    {
        //return GetComponent<Layer>();
        return LayerComponent();
    }

    TransformComponent Component::Transform()
    {
        //return GetComponent<Transform>();
        return TransformComponent();
    }
}
