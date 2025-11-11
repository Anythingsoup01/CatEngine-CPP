#include "Transform.h"
#include "Object.h"

namespace CatRuntime
{
    Vector3 Transform::Position()
    {
	    return Transform_GetPosition(Object.m_InstanceID);
    }
}
