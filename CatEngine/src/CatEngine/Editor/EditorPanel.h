#pragma once

#include "CatEngine/Event/Event.h"
#include "CatEngine/Scene/Scene.h"

namespace CatEngine {
class EditorPanel {
public:
  virtual ~EditorPanel() = default;

  virtual void OnImGuiRender(bool &isOpen) = 0;
  virtual void OnEvent(Event &e) {}
  virtual void SetSceneContext(const Ref<Scene> &context) {}
};
} // namespace CatEngine
