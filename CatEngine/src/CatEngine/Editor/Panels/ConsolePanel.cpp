#include "ConsolePanel.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "imgui.h"

namespace CatEngine {

ConsolePanel::ConsolePanel(const Ref<Scene> &context) {}

static std::vector<const char *> s_LogLevels = {"All",     "Trace", "Log",
                                                "Warning", "Error", "Critical"};

void ConsolePanel::OnImGuiRender(bool &isOpen) {
  if (ImGui::Begin("Console", &isOpen)) {

    float buttonWidth = 60.0f;
    float comboWidth = ImGui::CalcTextSize("Log Level   Critical  ").x;
    float spacing = 8.0f;
    float rightPadding = 10.0f;
    float totalWidth = buttonWidth + spacing + comboWidth;

    // Align the Clear button and Combo box to the top right
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalWidth - rightPadding);
    if (ImGui::Button("Clear", ImVec2(buttonWidth, 0))) {
      m_ConsoleBuffer.clear();
      m_TraceBuffer.clear();
      m_LogBuffer.clear();
      m_WarningBuffer.clear();
      m_ErrorBuffer.clear();
      m_CriticalBuffer.clear();
    }

    ImGui:: SameLine();

    if (ImGuiDraw::Combo("Log Level", m_LogLevel, s_LogLevels)) {
    }

    ImGui::Spacing();

    // TODO: Work on a status bar
    ImVec2 contentRegionSize =
        ImVec2(0, ImGui::GetContentRegionAvail().y - 15.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    if (ImGui::BeginChild("ConsoleLogRegion", contentRegionSize, true,
                          ImGuiWindowFlags_HorizontalScrollbar)) {

      // Lambda to render a buffer line-by-line with dynamic prefix coloring
      auto RenderParsedBuffer = [](const std::string &buffer) {
        std::stringstream ss(buffer);
        std::string line;
        while (std::getline(ss, line)) {
          if (line.empty())
            continue;

          if (line.rfind("[TRACE]:", 0) == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
          } else if (line.rfind("[LOG]:", 0) == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
          } else if (line.rfind("[WARNING]:", 0) == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
          } else if (line.rfind("[ERROR]:", 0) == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
          } else if (line.rfind("[CRITICAL]:", 0) == 0) {
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImVec4(0.5f, 0.0f, 0.0f, 1.0f)); // Dark red text
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
          } else {
            ImGui::TextUnformatted(line.c_str());
          }
        }
      };

      // Lambda for rendering a single-color buffer block efficiently
      auto RenderSolidBuffer = [](const std::string &buffer, ImVec4 color) {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(buffer.c_str());
        ImGui::PopStyleColor();
      };

      // 3. Render based on selected log level
      switch (m_LogLevel) {
      case 0: // All (Parses mixed logs)
        RenderParsedBuffer(m_ConsoleBuffer);
        break;
      case 1: // Trace (Green)
        RenderSolidBuffer(m_TraceBuffer, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        break;
      case 2: // Log (White)
        RenderSolidBuffer(m_LogBuffer, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        break;
      case 3: // Warning (Yellow)
        RenderSolidBuffer(m_WarningBuffer, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
        break;
      case 4: // Error (Red)
        RenderSolidBuffer(m_ErrorBuffer, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        break;
      case 5: // Critical (Dark Red text with Red background)
        RenderParsedBuffer(m_CriticalBuffer);
        break;
      default:
        RenderParsedBuffer(m_ConsoleBuffer);
        break;
      }
      static bool autoScroll = true;
      if (ImGui::GetScrollY() < ImGui::GetScrollMaxY()) {
        autoScroll = false;
      }
      if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        autoScroll = true;
      }

      // 2. If auto-scroll is active, force the scroll position to the bottom of the window
      if (autoScroll) {
        ImGui::SetScrollHereY(1.0f);
      }

    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
  }
  ImGui::End();
}

void ConsolePanel::WriteTrace(const std::string &trace) {
  std::string formatted = "[TRACE]: " + trace + "\n";
  m_ConsoleBuffer.append(formatted);
  m_TraceBuffer.append(formatted);
}

void ConsolePanel::WriteLog(const std::string &log) {
  std::string formatted = "[LOG]: " + log + "\n";
  m_ConsoleBuffer.append(formatted);
  m_LogBuffer.append(formatted);
}

void ConsolePanel::WriteWarning(const std::string &warning) {
  std::string formatted = "[WARNING]: " + warning + "\n";
  m_ConsoleBuffer.append(formatted);
  m_WarningBuffer.append(formatted);
}

void ConsolePanel::WriteError(const std::string &error) {
  std::string formatted = "[ERROR]: " + error + "\n";
  m_ConsoleBuffer.append(formatted);
  m_ErrorBuffer.append(formatted);
}

void ConsolePanel::WriteCritical(const std::string &critical) {
  std::string formatted = "[CRITICAL]: " + critical + "\n";
  m_ConsoleBuffer.append(formatted);
  m_CriticalBuffer.append(formatted);
}

} // namespace CatEngine
