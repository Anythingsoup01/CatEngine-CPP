#pragma once

#include <format>

#include "CatScriptCore/InternalCalls/InternalCalls.h"
#include "CatScriptCore/Core/Formatter.h"

namespace CatRuntime {

class Console {
public:
  Console() = default;
  ~Console() = default;

  template<typename... Args>
  static void Trace(std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Editor_ConsoleLog(1, formatted.c_str());
  }

  template<typename... Args>
  static void Log(std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Editor_ConsoleLog(2, formatted.c_str());
  }

  template<typename... Args>
  static void Warning(std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Editor_ConsoleLog(3, formatted.c_str());
  }

  template<typename... Args>
  static void Error(std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Editor_ConsoleLog(4, formatted.c_str());
  }

  template<typename... Args>
  static void Critical(std::format_string<Args...> fmt, Args&&... args) {
    std::string formatted = std::format(fmt, std::forward<Args>(args)...);
    Editor_ConsoleLog(5, formatted.c_str());
  }
};

}
