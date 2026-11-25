#include "PlayerController.h"

#include "CatScriptCore/Scene/Commands.h"
#include "CatScriptCore/Scene/Editor/Console.h"
#include "CatScriptCore/Types/Vector.h"

#include "PlayerHealth.h"


namespace CatRuntime {

PlayerController::PlayerController() {}

PlayerController::~PlayerController() {}

void PlayerController::Start() {
}

void PlayerController::FixedUpdate(float ts) {
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
  Console::Log("Player Velocity: X {} , Y {}", playerVelocity.X, playerVelocity.Y);
}

void PlayerController::OnCollisionEnter(Object other) {
  Console::Log("OTHER LAYER: {}", other.layer());

  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = true;
    m_CanJump = true;
  }
}

void PlayerController::OnCollisionExit(Object other) {
  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = false;
  }
}
void PlayerController::ApplyHealth(const float &health) {
  m_PlayerHealth += health;
  if (m_PlayerHealth > 100) {
    m_PlayerHealth = 100;
  } else if (m_PlayerHealth < 0) {
    m_PlayerHealth = 0;
  }
}
} // namespace CatRuntime
