#include "Object.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

#include "CircleRenderer.h"
#include "SpriteRenderer.h"
#include "Rigidbody2D.h"

#include <iostream>

#define Internal_Object_HasComponent(entityID, Type) Object_HasComponent(entityID, #Type)

namespace CatRuntime
{

    #define HasComponentOverload(Type, Instance) \
    template <> bool Object::HasComponent<Type>() { return Internal_Object_HasComponent(Instance, Type); }

    template<typename T> bool Object::HasComponent() { return false; }

    HasComponentOverload(Transform, m_InstanceID);
    HasComponentOverload(CircleRenderer, m_InstanceID);
    HasComponentOverload(SpriteRenderer, m_InstanceID);
    HasComponentOverload(Rigidbody2D, m_InstanceID);

    template<typename T> T Object::GetComponent() { return T(0); };
    
    #define GetComponentOverload(Type, Instance) \
    template<> \
    Type Object::GetComponent<Type>() \
    {\
        if (!HasComponent<Type>()) \
            return Type(0); \
        Type type = Type(m_InstanceID); \
        return type; \
    } \

    GetComponentOverload(Transform, m_InstanceID);
    GetComponentOverload(CircleRenderer, m_InstanceID);
    GetComponentOverload(SpriteRenderer, m_InstanceID);
    GetComponentOverload(Rigidbody2D, m_InstanceID);
}
