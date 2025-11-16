#include "PlayerController.h"

#include "CatScriptCore/Scene/Component/SpriteRenderer.h"

#include "signal.h"

namespace CatRuntime
{

    void PlayerController::Start()
    {
        m_Transform.position() = {0,0,0};

    }

    void PlayerController::Update(float ts)
    {
        m_RB2D.ApplyForce({0, m_PlayerSpeed}, {0, 0});
        std::cout << "Player Speed: " << m_PlayerSpeed << "\n";
    }

    void PlayerController::OnCollisionEnter(uint64_t other)
    {

        Object example(other);

        example.GetComponent<SpriteRenderer>().color() = {1, 0, 0, 1};
    }

    void PlayerController::OnCollisionExit(uint64_t other)
    {

    }

    PlayerController* PlayerController::Create(uint64_t entityID)
    {
        return new PlayerController(entityID);
    }

}
