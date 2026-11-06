#pragma once
#include "CatScriptCore/Scene/Component/Object.h"

namespace CatRuntime {
class GroundInteract : public Object {
public:
  void Start();
  void Update(float ts);

  void OnCollisionEnter(Object other);
  void OnCollisionExit(Object other);

private: // Be sure to put editor variables here, or in the public field!
};
} // namespace CatRuntime
