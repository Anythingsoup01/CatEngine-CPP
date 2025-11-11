#pragma once

#include "Object.h"

namespace CatRuntime
{
	class Component : public Object
	{
    public:

        Tag Tag()
        {
            return GetComponent<Tag>();
        }

		Layer Layer()
		{
			return GetComponent<Layer>();
		}

		Transform Transform()
		{	
            return GetComponent<Transform>();	
		}

	}
}
