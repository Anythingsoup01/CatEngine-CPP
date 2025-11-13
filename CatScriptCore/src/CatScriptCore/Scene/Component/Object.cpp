#include "Object.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Scene/Component/CircleRenderer.h"
#include "CatScriptCore/Scene/Component/Transform.h"

#include <iostream>

#define Internal_Object_HasComponent(entityID, Type) Object_HasComponent(entityID, #Type)

namespace CatRuntime
{

    Object::Object(uint64_t entityID)
    {
        m_InstanceID = entityID;
    }


    #define HasComponentOverload(Type, Instance) \
    template <> bool Object::HasComponent<Type>() { return Internal_Object_HasComponent(Instance, Type); }

    template<typename T> bool Object::HasComponent() { return false; }

    HasComponentOverload(Transform, m_InstanceID);
    HasComponentOverload(CircleRenderer, m_InstanceID);

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
}
