#pragma once
#include "CatScriptCore/ScriptInclude.h"
namespace CatRuntime {
class BoxSpinner : public Object {
public:
  virtual void Start();
  virtual void Update(float ts);

private: // Be sure to put editor variables here, or in the public field!
  int m_Test = 0;
};
} // namespace CatRuntime
