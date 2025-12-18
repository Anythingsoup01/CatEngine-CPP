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

    HasComponentOverload(CircleRenderer, m_EntityID);
    HasComponentOverload(SpriteRenderer, m_EntityID);
    HasComponentOverload(Rigidbody2D, m_EntityID);

    template<typename T> T Object::GetComponent() { return T(0); };
    
    #define GetComponentOverload(Type, Instance) \
    template<> \
    Type Object::GetComponent<Type>() \
    {\
        if (!HasComponent<Type>()) \
            return Type(0); \
        Type type = Type(Instance); \
        return type; \
    } \

    GetComponentOverload(CircleRenderer, m_EntityID);
    GetComponentOverload(SpriteRenderer, m_EntityID);
    GetComponentOverload(Rigidbody2D, m_EntityID);
}
