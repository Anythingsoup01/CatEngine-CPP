#include "PlayerHealth.h"
#include "CatScriptCore/Scene/Editor/Console.h"

namespace CatRuntime {

PlayerHealth::PlayerHealth() {
}
PlayerHealth::~PlayerHealth() {}

void PlayerHealth::Start() {}
void PlayerHealth::Update(float ts) {}

void PlayerHealth::DamagePlayer(const float &dmg) {
  m_Health -= dmg;
  if (m_Health <= 0) {
    m_Health = 0;
    Console::Log("GAME OVER!");
  }
}

void PlayerHealth::HealPlayer(const float &heal) {
  m_Health += heal;
  if (m_Health > m_MaxHealth) {
    m_Health = m_MaxHealth;
  }
}

} // namespace CatRuntime
