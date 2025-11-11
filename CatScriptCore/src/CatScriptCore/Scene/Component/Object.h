#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"

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

        /*
		Transform Transform()
		{
            return GetComponent<Transform>();
		}
        */

        template<typename T>
		bool HasComponent<T>()
		{
            static_assert(!Object_HasComponent, "Object_HasComponent is not defined!");
			T componentType = typeof(T);
			return Object_HasComponent(m_InstanceID, componentType);
		}
/*
		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Object = this };
			return component;
		}

		public Object FindObjectByName(string name)
		{
			ulong objectID = InternalCalls.Object_FindObjectByName(name);
			if (objectID == 0)
				return null;
			return new Object(objectID) { };
		}

		public T As<T>() where T : Object, new()
		{
			object instance = InternalCalls.GetScriptInstance(m_InstanceID);
			return instance as T;
		}
        */
	};
}
