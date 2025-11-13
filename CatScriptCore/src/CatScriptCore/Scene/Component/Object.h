#pragma once

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include <memory>

namespace CatRuntime
{


	class Object
	{
    public:
        Object() = default;
        Object(uint64_t entityID);
        virtual ~Object() = default;

        uint64_t GetInternalInstanceID() const { return m_InstanceID; }

        template<typename T>
        bool HasComponent();

        template<typename T>
        T GetComponent();

    protected :
        uint64_t m_InstanceID = 0;


    private:

    };

}
