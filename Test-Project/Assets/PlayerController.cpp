#include "PlayerController.h"

namespace CatRuntime
{
    void PlayerController::Start()
    {

    }

    void PlayerController::Update(float ts)
    {

    }

    PlayerController* PlayerController::Create(uint64_t entityID)
    {
        return new PlayerController(entityID);
    }
}
