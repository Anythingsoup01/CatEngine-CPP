#include "Transform.h"

namespace CatRuntime
{
    Vector3 Transform::Position()
    { 
        Vector3 pos;
        Transform_GetPosition(GetInternalInstanceID(), &pos);
        return pos;
    }
    
    void Transform::Position(const Vector3& pos)
    {
        Vector3 newPos = pos;
        Transform_SetPosition(GetInternalInstanceID(), &newPos);
    }
}
