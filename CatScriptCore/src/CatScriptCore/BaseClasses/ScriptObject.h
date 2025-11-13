#pragma once

#include "CatScriptCore/Types/Types.h"

#include "CatScriptCore/Scene/Component/Object.h"

namespace CatRuntime
{
    class ScriptObject : public Object
    {
    public:
        virtual void Start();
        virtual void Update(float ts);
        virtual void OnCollisionEnter(uint64_t other);
        virtual void OnCollisionExit(uint64_t other);

    protected:
    };
}
