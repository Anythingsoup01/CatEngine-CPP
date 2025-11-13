#include "Enemy.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Scene/Component/Transform.h"
#include "CatScriptCore/Scene/Component/CircleRenderer.h"

namespace CatRuntime
{

    void EnemyController::Start()
    {
        m_Transform = GetComponent<Transform>();
    }
    void EnemyController::Update(float ts)
    {
        Vector3 pos = m_Transform.Position();

        if (Input_IsKeyDown(65))
            pos.x -= 1 * ts;

        m_Transform.Position(pos);
    }

    EnemyController* EnemyController::Create(uint64_t entityID)
    {
        return new EnemyController(entityID);
    }

}
