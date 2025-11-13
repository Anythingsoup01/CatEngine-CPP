#include "PlayerController.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Scene/Component/Transform.h"
#include "CatScriptCore/Scene/Component/CircleRenderer.h"

namespace CatRuntime
{

    void PlayerController::Start()
    {

    }

    void PlayerController::Update(float ts)
    {

    }

    void PlayerController::OnCollisionEnter(uint64_t other)
    {

        std::cout << "other id "  << other << "\n";
        Object example(other);

        example.GetComponent<Transform>().Position({0, -5, 0});
    }

    void PlayerController::OnCollisionExit(uint64_t other)
    {

    }

    PlayerController* PlayerController::Create(uint64_t entityID)
    {
        return new PlayerController(entityID);
    }

}
