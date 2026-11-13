#pragma once

#include "CatEngine/Editor/EditorPanel.h"

#include "CatEngine/Scene/Scene.h"

namespace CatEngine {

class ConsolePanel : public EditorPanel {
public:
  ConsolePanel() = default;
  ConsolePanel(const Ref<Scene> &context);

  virtual void SetSceneContext(const Ref<Scene> &context) override { m_Context = context; }
  Ref<Scene> GetSceneContext() { return m_Context; }

  virtual void OnImGuiRender(bool &isOpen) override;
  virtual void OnEvent(Event &e) override {}

  void WriteTrace(const std::string &trace);
  void WriteLog(const std::string &log);
  void WriteWarning(const std::string &warning);
  void WriteError(const std::string &error);
  void WriteCritical(const std::string &critical);

private:
private:
  Ref<Scene> m_Context;
  int m_LogLevel = 0;
  std::string m_ConsoleBuffer;
  std::string m_TraceBuffer;
  std::string m_LogBuffer;
  std::string m_WarningBuffer;
  std::string m_ErrorBuffer;
  std::string m_CriticalBuffer;
};
}
