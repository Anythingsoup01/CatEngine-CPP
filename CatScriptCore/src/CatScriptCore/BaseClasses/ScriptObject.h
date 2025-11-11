#pragma once

#include "CatScriptCore/Types/Types.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatEngine
{
    class IScriptObject
    {
    public:
        virtual void Start() {}
        virtual void Update(float ts) {}
        //virtual void OnCollisionEnter(const Entity& other) {}
        //virtual void OnCollisionExit(const Entity& other) {}

    protected:
    };
}
