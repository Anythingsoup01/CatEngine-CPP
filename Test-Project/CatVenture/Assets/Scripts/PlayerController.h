#pragma once

#include "CatScriptCore/Types/Texture.h"
#include <CatScriptCore/ScriptInclude.h>

namespace CatRuntime
{
    class PlayerController : public Object
    {
    public:
        void Start();
        void Update(float ts);

        void OnCollisionEnter(uint64_t other);
        void OnCollisionExit(uint64_t other);

        static PlayerController* Create(uint64_t entityID);

    private:
        Object m_Entity;
        Texture2D m_CatTexture;

        float m_PlayerTerminalVelocity = 25.0f;
        float m_PlayerSpeed = 5;
        float m_JumpForce = 25.0f;

        bool m_Grounded = false;
        bool m_CanJump = false;

    };
}
