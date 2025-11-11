#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"

#include "Transform.h"

namespace CatRuntime
{
	class Object
	{
    public:
		Object() { m_InstanceID = 0; }

    protected :
		Object(ulong id)
		{
			m_InstanceID = id;
		}

		uint64_t m_InstanceID;

		Transform Transform()
		{
            return GetComponent<Transform>();
		}

        template<typename T>
		bool HasComponent<T>()
		{
            static_assert(!Object_HasComponent, "Object_HasComponent is not defined!");
			return Object_HasComponent(m_InstanceID, T);
		}

        template<typename T>
		T GetComponent<T>()
		{
			if (!HasComponent<T>())
            {
                printf("Component not found!\n");
				return T();
            }
            return Object_GetComponent(m_InstanceID);
		}

		Object FindObjectByName(std::string name)
		{
			ulong objectID = Object_FindObjectByName(m_InstanceID, name);
			if (objectID == 0)
            {
                printf("Object not found!\n");
				return Object();
            }
			return Object(objectID);
		}

        template<typename T>
		T As<T>()
        {
			T instance = GetScriptInstance(m_InstanceID);
			return instance;
		}
	};
}
