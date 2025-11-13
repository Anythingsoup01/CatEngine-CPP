#pragma once

#include "CatScriptCore/Types/Types.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

#include "CatScriptCore/Scene/Component/Object.h"

namespace CatRuntime
{
    class ScriptObject : public Object
    {
    public:
        virtual void Start();
        virtual void Update(float ts);

    protected:
    };
}
