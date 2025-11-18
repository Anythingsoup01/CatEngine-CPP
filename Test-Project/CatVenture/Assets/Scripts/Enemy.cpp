#include "Enemy.h"


namespace CatRuntime
{

    void EnemyController::Start()
    {
        transform.position() = {0, 0, 0};
    }
    void EnemyController::Update(float ts)
    {
        Vector3 pos = transform.position();

        transform.position() = pos;
    }

    EnemyController* EnemyController::Create(uint64_t entityID)
    {
        return new EnemyController(entityID);
    }

}
