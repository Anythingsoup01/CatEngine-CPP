#pragma once

#include <CatScriptCore/ScriptInclude.h>

namespace CatRuntime {
class __attribute__((visibility("default"))) PlayerController : public Object {
public:
  PlayerController();
  ~PlayerController();

  void Start();
  void Update(float ts);

  void OnCollisionEnter(Object other);
  void OnCollisionExit(Object other);

public:
  float m_PlayerTerminalVelocity = 25.0f;
  float m_PlayerSpeed = 10;
  float m_JumpForce = 25.0f;

  float m_SpeedScale = 0.001f;

  bool m_Grounded = false;
  bool m_CanJump = false;
};
} // namespace CatRuntime
