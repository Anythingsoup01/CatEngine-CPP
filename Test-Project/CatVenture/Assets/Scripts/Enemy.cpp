#include "Enemy.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Scene/Component/Transform.h"
#include "CatScriptCore/Scene/Component/CircleRenderer.h"

namespace CatRuntime
{

    void EnemyController::Start()
    {
        transform.position() = {0, 0, 0};
    }
    void EnemyController::Update(float ts)
    {
        Vector3 pos = transform.position();

        if (Input_IsKeyDown(65))
            pos.x -= 1 * ts;

        transform.position() = pos;
    }

    EnemyController* EnemyController::Create(uint64_t entityID)
    {
        return new EnemyController(entityID);
    }

}
