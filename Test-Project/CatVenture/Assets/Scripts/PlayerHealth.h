#pragma once

#include "CatScriptCore/ScriptInclude.h"

namespace CatRuntime {

class __attribute__((visibility("default"))) PlayerHealth : public Object {
public:
  PlayerHealth();
  ~PlayerHealth();

  void Start();
  void Update(float ts);

private:
  void DamagePlayer(const float &dmg);
  void HealPlayer(const float &heal);

public:
  float m_Health = 100.0f;
  float m_MaxHealth = 100.0f;

private:
  friend class PlayerController;
};

}
