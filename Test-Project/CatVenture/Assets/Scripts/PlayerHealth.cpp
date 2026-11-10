#include "PlayerHealth.h"

namespace CatRuntime {

PlayerHealth::PlayerHealth() {}
PlayerHealth::~PlayerHealth() {}

void PlayerHealth::Start() {}
void PlayerHealth::Update(float ts) {}

void PlayerHealth::DamagePlayer(const float &dmg) {
  m_Health -= dmg;
  if (m_Health <= 0) {
    m_Health = 0;
    std::cout << "GAME OVER!\n";
  }
}

void PlayerHealth::HealPlayer(const float &heal) {
  m_Health += heal;
  if (m_Health > m_MaxHealth) {
    m_Health = m_MaxHealth;
  }
}

} // namespace CatRuntime
