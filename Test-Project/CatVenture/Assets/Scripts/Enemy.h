#pragma once

#include <CatScriptCore/ScriptIncludes.h>

#include "CatScriptCore/Scene/Component/Transform.h"

namespace CatRuntime
{
    class EnemyController : public ScriptObject
    {
    public:
        EnemyController(uint64_t entityID) { m_InstanceID = entityID; }
        virtual void Start() override;
        virtual void Update(float ts) override;

        static EnemyController* Create(uint64_t entityID);

    private:
#   define PUBLIC

        Transform m_Transform;

        //TransformComponent m_Test;
        //TransformComponent m_Test3;


#   define PRIVATE
        //Rigidbody2DComponent m_RB2D;
    };
}
