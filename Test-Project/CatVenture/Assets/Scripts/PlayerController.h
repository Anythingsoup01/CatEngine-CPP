#pragma once

#include <CatScriptCore/ScriptInclude.h>

#include <CatScriptCore/Asset/Prefab.h>

#include "PlayerHealth.h"

namespace CatRuntime {
class __attribute__((visibility("default"))) PlayerController : public Object {
public:
  PlayerController();
  ~PlayerController();

  void Start();
  void FixedUpdate(float ts);

  void OnCollisionEnter(Object other);
  void OnCollisionExit(Object other);

  void ApplyHealth(const float &health);

public:
  float m_PlayerHealth = 0.0f;
  float m_PlayerTerminalVelocity = 25.0f;
  float m_PlayerSpeed = 10;
  float m_JumpForce = 25.0f;

  float m_SpeedScale = 0.001f;

  bool m_Grounded = false;
  bool m_CanJump = false;
};
} // namespace CatRuntime
