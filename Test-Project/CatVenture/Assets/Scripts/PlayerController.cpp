#include "PlayerController.h"

namespace CatRuntime
{

    void PlayerController::Start()
    {
        m_RB2D = GetComponent<Rigidbody2D>();
    }

    void PlayerController::Update(float ts)
    {

        Vector2 playerVelocity = {0, 0};

        if (m_Grounded)
        {
            if (Input::KeyPressed(KeyCode::A))
            {
                if (playerVelocity.x > -m_PlayerTerminalVelocity)
                    playerVelocity.x += -m_PlayerSpeed;
            }
            if (Input::KeyPressed(KeyCode::D))
            {
                if (playerVelocity.x < m_PlayerTerminalVelocity)
                    playerVelocity.x += m_PlayerSpeed;
            }
            if (Input::KeyPressed(KeyCode::Space))
            {
                if (m_CanJump)
                {
                    playerVelocity.y = m_JumpForce;
                    m_CanJump = false;
                }
            }
        }
        m_RB2D.ApplyLinearImpulse(playerVelocity, {0, 0});

    }

    void PlayerController::OnCollisionEnter(uint64_t other)
    {
        Object oth(other);

        std::cout << "OTHER LAYER: " << oth.layer.layer() << "\n";

        if (oth.layer.layer() == "Ground" && !m_CanJump)
        {
            m_Grounded = true;
            m_CanJump = true;
        }


    }

    void PlayerController::OnCollisionExit(uint64_t other)
    {
        Object oth(other);
        if (oth.layer.layer() == "Ground" && !m_CanJump)
        {
            m_Grounded = false;
        }

    }

    PlayerController* PlayerController::Create(uint64_t entityID)
    {
        return new PlayerController(entityID);
    }

}
