#pragma once

#include <CatScriptCore/ScriptInclude.h>

namespace CatRuntime
{
    class PlayerController : public ScriptObject
    {
    public:
        int m_Score;
    public:
        PlayerController(uint64_t entityID) { SetInstanceID(entityID); }
        void Start() override;
        void Update(float ts) override;

        static PlayerController* Create(uint64_t entityID);
    };
}
