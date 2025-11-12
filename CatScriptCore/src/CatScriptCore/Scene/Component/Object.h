#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
	class Object
	{
    public:
		Object() { m_InstanceID = 0; }

        void SetInstanceID(uint64_t id);
    protected :


    private:

		uint64_t m_InstanceID;
    };

}
