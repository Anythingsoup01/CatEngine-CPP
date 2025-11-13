#include "PlayerController.h"

#include "CatScriptCore/Scene/Component/Transform.h"

namespace CatRuntime
{

    void PlayerController::Start()
    {
        if (HasComponent<TransformComponent>())
        {
            std::cout << "TRANSFORM FOUND!\n";
        }
    }
    void PlayerController::Update(float ts)
    {

    }

    PlayerController* PlayerController::Create(uint64_t entityID)
    {
        return new PlayerController(entityID);
    }

}
