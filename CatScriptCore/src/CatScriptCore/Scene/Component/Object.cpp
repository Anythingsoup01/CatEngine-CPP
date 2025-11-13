#include "Object.h"
#include "CatScriptCore/Scene/Component/Transform.h"
#define Internal_Object_HasComponent(entityID, Type) Object_HasComponent(entityID, #Type)

namespace CatRuntime
{
    void Object::SetInstanceID(uint64_t id)
    {
        if (m_InstanceID != 0)
        {
            printf("ERROR: INSTANCE ID IS ALREADY SET!\n");
            return;
        }

        m_InstanceID = id;
    }

    template<typename T>
    bool Object::HasComponent()
    {
        return Internal_Object_HasComponent(m_InstanceID, T);
    }

    template<>
    bool Object::HasComponent<TransformComponent>()
    {
        return Internal_Object_HasComponent(m_InstanceID, "TransformComponent");
    }
}
