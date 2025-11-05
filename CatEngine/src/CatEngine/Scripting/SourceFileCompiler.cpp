
#include "CatEngine/Core/Log.h"
#include "cepch.h"
#include "SourceFileCompiler.h"

#include "CatEngine/Core/Application.h"
#include <sstream>
#include <vector>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{
    void SourceFileCompiler::AddFile(const std::filesystem::path& filePath)
    {
        std::string extension = filePath.extension();
        std::string fileName = filePath.filename();

        if (strncmp(extension.c_str(), ".cpp", 4) != 0)
            return;

        size_t fileNameLen = fileName.length();

        for (const auto& entry : std::filesystem::recursive_directory_iterator(Project::GetAssetDirectory()))
        {
            std::string path = entry.path().string();
            size_t hidden = path.find(".build");

            if (entry.is_regular_file() && hidden == std::string::npos)
            {
                std::string entryFileName = entry.path().filename();
                size_t entryFileNameLen = entryFileName.length();

                if (entryFileNameLen != fileNameLen)
                    continue;

                if (strncmp(entryFileName.c_str(), fileName.c_str(), fileNameLen) != 0)
                    continue;

                CompileFile(entry.path());
            }

        }

    }


    void SourceFileCompiler::AddDirectory(const std::filesystem::path& directory) 
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) 
        {
            std::string path = entry.path().string();
            size_t hidden = path.find(".build");

            if (entry.is_regular_file() && hidden == std::string::npos)
            {
                if (entry.path().extension() == ".cpp")
                {
                    CompileFile(entry.path());
                }
            }

        }
    }

    
    std::string SourceFileCompiler::GetBuildCommandVariables(const std::filesystem::path& filePath)
    {

        std::filesystem::path rootCatEnginePath = Application::Get().GetMainPath();
        std::string CatEngineSRC = rootCatEnginePath.string();
        CatEngineSRC.append("/CatEngine/src");
        std::string CatEngineVND = rootCatEnginePath.string();
        CatEngineVND.append("/vendor");

        std::filesystem::path sourceFilePath = filePath;
        std::filesystem::path compilePath = Project::GetAssetDirectory() / ".build" / filePath.filename();
        compilePath.replace_extension(".so");

        std::stringstream ss;
        ss << "gcc " << sourceFilePath << " -o " << compilePath << " \\\n"
           << "-I" << CatEngineSRC << " \\\n"
           << "-I" << CatEngineVND << "/spdlog/include \\\n"
           << "-I" << CatEngineVND << "/glm \\\n"
           << "-I" << CatEngineVND << "/entt \\\n"
           << "-I" << CatEngineVND << "/Glad/include \\\n"
           << "-I" << CatEngineVND << "/Box2D/include \\\n"
           << "-DCE_SCRIPT_COMPILATION \\\n"
           << "-fPIC -shared  -L" << rootCatEnginePath.string() << "/build/CatEngine -lCatEngine -ldl -lstdc++ -gdwarf-4\n";
        
        return ss.str();

    }

    void SourceFileCompiler::CompileFile(const std::filesystem::path& filePath)
    {
        CE_API_CRITICAL("COMPILING");
        std::stringstream ss;
        ss << GetBuildCommandVariables(filePath);
        CE_API_WARN(ss.str());
        system(ss.str().c_str());
    }




    void SourceFileCompiler::Init()
    {
        s_ProjectAssetPath = Project::GetAssetDirectory();
        AddDirectory(Project::GetAssetDirectory());
    }

}
