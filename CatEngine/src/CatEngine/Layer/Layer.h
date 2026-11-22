#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Event/Event.h"

namespace CatEngine {
class Layer {
public:
  Layer(const std::string &name = "Layer");
  virtual ~Layer() = default;

  virtual void OnAttach() = 0;
  virtual void OnDetach() = 0;
  virtual void OnUpdate(Time deltaTime) = 0;
  virtual void OnImGuiDraw() = 0;
  virtual void OnEvent(Event &e) = 0;

  virtual void NewSampleScene(const std::filesystem::path &loadPath) = 0;
  virtual void NewEmptyScene(const std::filesystem::path &loadPath) = 0;

  virtual void OpenScene(const AssetHandle &handle) = 0;

  virtual void OpenPreviousScene() = 0;

  inline const std::string &GetName() const { return m_DebugName; }

protected:
  std::string m_DebugName;
};
} // namespace CatEngine
