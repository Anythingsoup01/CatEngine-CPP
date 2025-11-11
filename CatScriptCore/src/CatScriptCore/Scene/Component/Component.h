#pragma once

#include "Object.h"

#include "Tag.h"
#include "Layer.h"
#include "Transform.h"

namespace CatRuntime
{
	class Component : public Object
	{
    public:
        TagComponent Tag();
		LayerComponent Layer();
        TransformComponent Transform();
    };
}
