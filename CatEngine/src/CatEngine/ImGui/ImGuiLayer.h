#pragma once

#include "CatEngine/Event/ApplicationEvent.h"
#include "CatEngine/Event/KeyEvent.h"
#include "CatEngine/Event/MouseEvent.h"
#include "CatEngine/Layer/Layer.h"

namespace CatEngine {

class ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  ~ImGuiLayer() = default;

  virtual void OnAttach() override;
  virtual void OnDetach() override;
  virtual void OnUpdate(Time deltaTime) override;
  virtual void OnImGuiDraw() override {};
  virtual void OnEvent(Event &event) override;

  virtual void NewSampleScene(const std::filesystem::path &loadPath) override {}
  virtual void NewEmptyScene(const std::filesystem::path &loadPath) override {}

  virtual void OpenScene(const AssetHandle &handle) override {}

  virtual void OpenPreviousScene() override {}


  void Begin();
  void End();

  bool BlockKeyboardEvents(bool block) {
    m_BlockKeyboardEvents = block;
    return block;
  }
  bool BlockMouseEvents(bool block) {
    m_BlockMouseEvents = block;
    return block;
  }
  void SetDarkThemeColors();
  void SetDefaultEditorStyle();

private:
  bool m_BlockKeyboardEvents = true;
  bool m_BlockMouseEvents = true;
  float m_Time = 0.0;
};

} // namespace CatEngine
