#pragma once

#include <CatScriptCore/ScriptIncludes.h>
#include "CatScriptCore/Scene/Component/Rigidbody2D.h"

namespace CatRuntime
{
    class PlayerController : public ScriptObject
    {
    public:
        PlayerController(uint64_t entityID) : ScriptObject(entityID), m_RB2D(entityID) {}
        virtual void Start() override;
        virtual void Update(float ts) override;
        virtual void OnCollisionEnter(uint64_t other) override;
        virtual void OnCollisionExit(uint64_t other) override;


        static PlayerController* Create(uint64_t entityID);

    private:
#   define PUBLIC
        float m_PlayerHealth = 15.0f;
        float m_PlayerSpeed = 25.0f;

        Rigidbody2D m_RB2D;

        Transform m_Transform;

        //TransformComponent m_Test;
        //TransformComponent m_Test3;


#   define PRIVATE
        //Rigidbody2DComponent m_RB2D;
    };
}
