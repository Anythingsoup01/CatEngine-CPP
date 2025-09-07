#include "cepch.h"
#include "System.h"

namespace CatEngine
{
    bool System::OpenFile(const std::filesystem::path& filePath)
    {
        #ifdef __linux__
        std::stringstream command;
        command << "xdg-open \"" << filePath << "\"";
        return system(command.str().c_str());
        #endif
    }
}
