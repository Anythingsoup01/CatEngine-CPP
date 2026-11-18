#include "PlayerController.h"

#include "CatScriptCore/Scene/Commands.h"
#include "CatScriptCore/Scene/Editor/Console.h"
#include "CatScriptCore/Types/Vector.h"

#include "PlayerHealth.h"


namespace CatRuntime {

PlayerController::PlayerController() {}

PlayerController::~PlayerController() {}

void PlayerController::Start() {
  if (m_PlayerPrefab) {
    for (int i = 0; i < 10; i++) {
      Object prefab = Scene::Instantiate(m_PlayerPrefab);
      if (prefab) {
        prefab.transform().local_position() = Vector3(i + 1, i + 1, 0);
        m_Enemies.push_back(prefab);
      }
    }
  }
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

  if (other.layer() == "Enemy") {
    for (auto it = m_Enemies.begin(); it != m_Enemies.end(); it++) {
      Object o = *it;
      if (o == other) {
        m_Enemies.erase(it);
        break;
      }
    }

    Scene::Destroy(other);
  }
}

void PlayerController::OnCollisionExit(Object other) {
  if (other.layer() == "Ground" && !m_CanJump) {
    m_Grounded = false;
  }
}
} // namespace CatRuntime
