#pragma once

#include <CatScriptCore/ScriptInclude.h>

namespace CatRuntime {
class __attribute__((visibility("default"))) EnemyController : public Object {
public:
  EnemyController();
  ~EnemyController();

  void Start();
  void Update(float ts);
};
} // namespace CatRuntime
