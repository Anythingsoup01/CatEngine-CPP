#include "PlayerController.h"
#include "CatScriptCore/Types/Vector.h"

#include "CatScriptCore/Scene/Component/AllComponents.h"
#include "PlayerHealth.h"

namespace CatRuntime {

PlayerController::PlayerController() {}

PlayerController::~PlayerController() {}

void PlayerController::Start() {
  m_PlayerHealth->DamagePlayer(100);
}

void PlayerController::Update(float ts) {
  Vector2 playerVelocity = {0, 0};

  if (m_Grounded) {
    if (Input::KeyPressed(KeyCode::A)) {
      if (playerVelocity.X > -m_PlayerTerminalVelocity)
        playerVelocity.X += -m_PlayerSpeed * m_SpeedScale;
    }
    if (Input::KeyPressed(KeyCode::D)) {
      if (playerVelocity.X < m_PlayerTerminalVelocity)
        playerVelocity.X += m_PlayerSpeed * m_SpeedScale;
    }
    if (Input::KeyPressed(KeyCode::Space)) {
      if (m_CanJump) {
        playerVelocity.Y = m_JumpForce;
        m_CanJump = false;
      }
    }
  }
  auto rb2d = GetComponent<Rigidbody2D>();
  rb2d.ApplyLinearImpulse(playerVelocity, {0, 0});
}

void PlayerController::OnCollisionEnter(Object other) {
  std::cout << "OTHER LAYER: " << other.layer() << "\n";

  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = true;
    m_CanJump = true;
  }

  // TODO: Make it to where this can be a member
  if (other.layer() == "Enemy") {
    PlayerHealth *ph = GetScript<PlayerHealth>();
    ph->DamagePlayer(50.0f);
  }

}

void PlayerController::OnCollisionExit(Object other) {
  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = false;
  }
}
} // namespace CatRuntime
