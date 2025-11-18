#pragma once
#include "CatScriptCore/ScriptInclude.h"
namespace CatRuntime
{
    class CatController : public ScriptObject
    {
    public:
        CatController(uint64_t entityID) : ScriptObject(entityID) {}
        virtual void Start() override;
        virtual void Update(float ts) override;
        static CatController* Create(uint64_t);
    private:    // Be sure to put editor variables here, or in the public field!
    };
}