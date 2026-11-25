#pragma once

#include "CatScriptCore/Scene/Component/Object.h"

namespace CatRuntime {

class CollectableItem : public Object {
public:
  CollectableItem();
  ~CollectableItem();

  void Start();

  void OnTriggerEnter(Object other);
  void OnTriggerExit(Object other);
public:
  float m_HealthModification = 100.0f;
};

}
