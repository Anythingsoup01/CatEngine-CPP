#pragma once

#include <string>
namespace CatRuntime {

class Console {
public:
  Console() = default;
  ~Console() = default;

  static void Trace(const std::string &msg);
  static void Log(const std::string &msg);
  static void Warning(const std::string &msg);
  static void Error(const std::string &msg);
  static void Critical(const std::string &msg);
};

}
