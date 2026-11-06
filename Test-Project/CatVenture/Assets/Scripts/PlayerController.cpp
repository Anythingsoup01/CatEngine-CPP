#include "PlayerController.h"
#include "CatScriptCore/Types/Vector.h"

#include "CatScriptCore/Scene/Component/AllComponents.h"

namespace CatRuntime {

PlayerController::PlayerController() {}

PlayerController::~PlayerController() {}

void PlayerController::Start() {
}

void PlayerController::Update(float ts) {
  Vector2 playerVelocity = {0, 0};

  if (m_Grounded) {
    if (Input::KeyPressed(KeyCode::A)) {
      if (playerVelocity.x > -m_PlayerTerminalVelocity)
        playerVelocity.x += -m_PlayerSpeed * m_SpeedScale;
    }
    if (Input::KeyPressed(KeyCode::D)) {
      if (playerVelocity.x < m_PlayerTerminalVelocity)
        playerVelocity.x += m_PlayerSpeed * m_SpeedScale;
    }
    if (Input::KeyPressed(KeyCode::Space)) {
      if (m_CanJump) {
        playerVelocity.y = m_JumpForce;
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

  if (other.HasComponent<SpriteRenderer>()) {
    auto sr = other.GetComponent<SpriteRenderer>();
    sr.color() = Vector4{1, 1, 0, 1};
  }
}

void PlayerController::OnCollisionExit(Object other) {
  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = false;
  }

  if (other.HasComponent<SpriteRenderer>()) {
    auto sr = other.GetComponent<SpriteRenderer>();
    sr.color() = Vector4{1, 1, 1, 1};
  }

}
} // namespace CatRuntime
