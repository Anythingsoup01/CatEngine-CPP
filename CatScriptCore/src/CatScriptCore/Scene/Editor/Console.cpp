#include "Console.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime {

void Console::Trace(const std::string &msg)     { Editor_ConsoleLog(1, msg.c_str()); }
void Console::Log(const std::string &msg)       { Editor_ConsoleLog(2, msg.c_str()); }
void Console::Warning(const std::string &msg)   { Editor_ConsoleLog(3, msg.c_str()); }
void Console::Error(const std::string &msg)     { Editor_ConsoleLog(4, msg.c_str()); }
void Console::Critical(const std::string &msg)  { Editor_ConsoleLog(5, msg.c_str()); }

} // namespace CatRuntime
