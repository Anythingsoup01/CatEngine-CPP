#pragma once

#include <CatScriptCore/ScriptIncludes.h>

namespace CatRuntime
{
    class PlayerController : public ScriptObject
    {
    public:
        PlayerController(uint64_t entityID) { SetInstanceID(entityID); }
        virtual void Start() override;
        virtual void Update(float ts) override;

        static PlayerController* Create(uint64_t entityID);
    private:
#   define PUBLIC
        float m_PlayerHealth = 15.0f;
        float m_PlayerSpeed = 25.0f;

        //TransformComponent* m_TransformComponent = nullptr;

        //TransformComponent m_Test;
        //TransformComponent m_Test3;


#   define PRIVATE
        //Rigidbody2DComponent m_RB2D;
    };
}
