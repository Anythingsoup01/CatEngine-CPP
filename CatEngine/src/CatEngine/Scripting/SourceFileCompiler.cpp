
#include "cepch.h"
#include "SourceFileCompiler.h"

#include "CatEngine/Core/Application.h"
#include <sstream>
#include <vector>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{
    bool InLine(const char* line, const char* word)
    {
        size_t len = strlen(word);
        return strncmp(line, word, len) == 0;
    }

    bool IsVariable(const char* line)
    {
        for (auto& keyword : s_Keywords)
        {
            if (InLine(line, keyword))
                return true;
        }

        return false;
    }


    KeyWordType GetKeyWord(const char* line)
    {
        for (auto& keyword : s_Keywords)
        {
            if (InLine(line, keyword))
            {
                if (InLine(keyword, "char"))
                    return KeyWordType::Char;
                if (InLine(keyword, "float") || InLine(keyword, "double") || InLine(keyword, "int16_t") || InLine(keyword, "int32_t") || InLine(keyword, "int") || InLine(keyword, "int64_t") || InLine(keyword, "uint16_t") || InLine(keyword, "uint32_t") || InLine(keyword, "unsigned int") || InLine(keyword, "uint64_t"))
                    return KeyWordType::Digit;
                if (InLine(keyword, "bool"))
                    return KeyWordType::Bool;
                if (InLine(keyword, "std::string"))
                    return KeyWordType::String;
                if (InLine(keyword, "glm::vec2"))
                    return KeyWordType::Vec2;
                if (InLine(keyword, "glm::vec3"))
                    return KeyWordType::Vec3;
                if (InLine(keyword, "glm::vec4"))
                    return KeyWordType::Vec4;
                if (InLine(keyword, "Rigidbody2DComponent"))
                    return KeyWordType::Rigidbody2DComponent;
                if (InLine(keyword, "TransformComponent"))
                    return KeyWordType::TransformComponent;
            }

        }
        return KeyWordType::None;
    }

    bool NeedsDefaultVariable(const std::string& line, std::string& defaultVariable)
    {
        size_t equalSign = line.find("=");
        if (equalSign == std::string::npos)
        {
            KeyWordType keyword = GetKeyWord(line.c_str());
            switch (keyword) 
            {
                case KeyWordType::Char:
                    defaultVariable = " = \'\';";
                    return true;
                case KeyWordType::Digit:
                    defaultVariable = " = 0;";
                    return true;
                case KeyWordType::Bool:
                    defaultVariable = " = false;";
                    return true;
                case KeyWordType::String:
                    defaultVariable = " = \"\";";
                    return true;
                case KeyWordType::Vec2:
                    defaultVariable = " = glm::vec2(0);";
                    return true;
                case KeyWordType::Vec3:
                    defaultVariable = " = glm::vec3(0);";
                    return true;
                case KeyWordType::Vec4:
                    defaultVariable = " = glm::vec4(0);";
                    return true;
                case KeyWordType::TransformComponent:
                    defaultVariable = " = TransformComponent();";
                    return true;
                case KeyWordType::Rigidbody2DComponent:
                    defaultVariable = " = Rigidbody2DComponent();";
                    return true;

                default: break;
            }
        }
        defaultVariable = ";";
        return false;
    }

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

                CopyAndPrepareFile(entry.path());

            }

        }

    }
    void SourceFileCompiler::RemoveFile(const std::filesystem::path& filePath)
    {
        bool found = false;

        int index = 0;
        for (auto& fd : m_CompiledFiles)
        {
            if (fd.SourceFilePath == filePath)
            {
                found = true;
            }
            index++;
        }

        if (!found)
        {
            CE_API_ERROR("Failed to locate and remove '{}'", filePath.string());
            return;
        }
        m_CompiledFiles.erase(m_CompiledFiles.begin() + index);
    }

    void SourceFileCompiler::AddDirectory(const std::filesystem::path& directory) 
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) 
        {
            std::string path = entry.path().string();
            size_t hidden = path.find(".build");

            if (entry.is_regular_file() && hidden == std::string::npos)
            {
                auto& filePath = entry.path();
                std::string extension = filePath.extension();
                std::string fileName = filePath.filename();

                if (strncmp(extension.c_str(), ".cpp", 4) == 0)
                {
                    CopyAndPrepareFile(filePath, true);
                }

            }

        }
    }

    void SourceFileCompiler::CopyAndPrepareFile(const std::filesystem::path& filePath, bool initialLoad)
    {
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            CE_API_ERROR("Failed to open file : {}", filePath.string());
            return;
        }

        std::stringstream copy; copy << in.rdbuf();

        in.close();

        std::ofstream out(filePath, std::ios::out | std::ios::trunc);
        if (!out.is_open())
        {
            CE_API_ERROR("Failed to open file : {}", filePath.string());
            return;
        }

        std::string line;
        bool inClass = false;
        bool firstLoopInClass = true;

        int openingBrackets = 0, closingBrackets = 0;
        std::string className;

        while (std::getline(copy, line))
        {
            std::string lineParse = line;
            lineParse.erase(remove_if(lineParse.begin(), lineParse.end(), isspace), lineParse.end());

            if (strncmp(lineParse.c_str(), "class", 5) == 0)
            {
                inClass = true;
                size_t eow = lineParse.find(":", 5);
                className = lineParse.substr(5, eow - 5);
            }

            if (inClass)
            {
                if (lineParse.find("{") != std::string::npos)
                    openingBrackets++;
                if (lineParse.find("}") != std::string::npos)
                    closingBrackets++;

                if (openingBrackets == closingBrackets && !firstLoopInClass)
                    inClass = false;


                // Keep at end!
                firstLoopInClass = false;
            }
            else
            {
                if (IsVariable(lineParse.c_str()))
                {
                    std::stringstream ss;

                    line.erase(line.length() - 1);
                    ss << "extern \"C\" " << line;
                    std::string defaultVariable;
                    NeedsDefaultVariable(line, defaultVariable);
                    ss << defaultVariable;
                    line = ss.str();
                }
            }

            out << line << "\n";
        }

        out << "extern \"C\" CatEngine::IScriptObject* create() { return new " << className << "; }\n"
            << "extern \"C\" void destroy(CatEngine::IScriptObject* script) { delete script; }";

        out.close();

        std::string fileName = filePath.filename();
        fileName.erase(fileName.length() - 4);

        std::string libFilePath = ".build/lib";
        libFilePath.append(fileName).append(".so");
        std::filesystem::path sharedObjectPath = s_ProjectAssetPath / libFilePath;

        std::string objectFilePath = ".build/";
        objectFilePath.append(fileName).append(".o");
        std::filesystem::path objectPath = s_ProjectAssetPath / objectFilePath;

        FileDescription fd;
        fd.Name = fileName;
        fd.SharedObjectPath = sharedObjectPath;
        fd.ObjectPath = objectPath;
        fd.SourceFilePath = filePath;

        std::ifstream check(sharedObjectPath, std::ios::in | std::ios::binary);
        if (!check.is_open() || !initialLoad)
        {
            CompileFile(fd);
        }
        else
        {
            m_CompiledFiles.push_back(fd);
        }


        std::ofstream out1(filePath, std::ios::out | std::ios::trunc);
        if (!out1.is_open())
        {
            CE_API_ERROR("Failed to open file : {}", filePath.string());
            return;
        }

        out1 << copy.str();

        out1.close();


    }

    
    std::string SourceFileCompiler::GetBuildCommandVariables(const FileDescription& fd)
    {

        std::filesystem::path rootCatEnginePath = Application::Get().GetMainPath();
        std::string CatEngineSRC = rootCatEnginePath.string();
        CatEngineSRC.append("/CatEngine/src");
        std::string CatEngineVND = rootCatEnginePath.string();
        CatEngineVND.append("/vendor");

        std::stringstream ss;
        ss << "gcc -c " << fd.SourceFilePath.string() << " -o " << fd.ObjectPath.string() << " \\\n"
           << "-I" << CatEngineSRC << " \\\n"
           << "-I" << CatEngineVND << "/spdlog/include \\\n"
           << "-I" << CatEngineVND << "/glm \\\n"
           << "-I" << CatEngineVND << "/entt \\\n"
           << "-I" << CatEngineVND << "/Glad/include \\\n"
           << "-DCE_SCRIPT_COMPILATION \\\n"
           << "-fPIC; \n";
     

        ss << "gcc -shared " << fd.ObjectPath << " -o " << fd.SharedObjectPath << " -L" << rootCatEnginePath.string() << "/build/CatEngine -lCatEngine -ldl -lstdc++\\\n";
        
        return ss.str();

    }

    void SourceFileCompiler::CompileFile(FileDescription fd)
    {
        std::stringstream ss;
        ss << "cd " << Project::GetAssetDirectory() << "; " << GetBuildCommandVariables(fd);
        system(ss.str().c_str());

        bool fileExists = false;

        for (auto& file : m_CompiledFiles)
        {
            if (fd.Name == file.Name)
            {
                fileExists = true;
                break;
            }
        }

        if (!fileExists)
            m_CompiledFiles.push_back(fd);

    }


    void SourceFileCompiler::Init()
    {
        s_ProjectAssetPath = Project::GetAssetDirectory();
    }

}
