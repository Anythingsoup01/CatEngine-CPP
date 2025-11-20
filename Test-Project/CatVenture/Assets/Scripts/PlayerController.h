#pragma once

#include "CatScriptCore/Types/Texture.h"
#include <CatScriptCore/ScriptInclude.h>

namespace CatRuntime
{
    class PlayerController : public ScriptObject
    {
    public:
        PlayerController(uint64_t entityID) : ScriptObject(entityID) {}

        virtual void Start() override;
        virtual void Update(float ts) override;

        virtual void OnCollisionEnter(uint64_t other) override;
        virtual void OnCollisionExit(uint64_t other) override;

        static PlayerController* Create(uint64_t entityID);

    private:
#   define PUBLIC
        float m_PlayerTerminalVelocity = 25.0f;
        float m_PlayerSpeed = 5;

        Rigidbody2D m_RB2D;

        Transform m_RedTransform;

        Texture2D m_CatTexture;

        bool m_CanJump = false;
        bool m_Grounded = false;

        float m_JumpForce = 25.0f;

#   define PRIVATE
    };
}
