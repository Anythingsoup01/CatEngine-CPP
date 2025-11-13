#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"

#define Internal_Object_HasComponent(entityID, Type) Object_HasComponent(entityID, #Type)

namespace CatRuntime
{
	class Object
	{
    public:
		Object() { m_InstanceID = 0; }

        void SetInstanceID(uint64_t id);
        const uint64_t& GetInternalInstanceID() const { return m_InstanceID; }
    protected :

        template<typename T>
        bool HasComponent();


    private:

		uint64_t m_InstanceID;
    };

}
