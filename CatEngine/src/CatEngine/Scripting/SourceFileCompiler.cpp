
#include "cepch.h"
#include "SourceFileCompiler.h"

#include "CatEngine/Core/Application.h"
#include <sstream>
#include <vector>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{

    enum class KeyWordType
    {
        None = 0,
        Digit, String, Char, Bool, Vec2, Vec3, Vec4,
        TransformComponent, Rigidbody2DComponent
    };

    static std::vector<const char*> s_Keywords =
    {
        "float", "double",
		"char",
		"int16_t", "int32_t", "int", "int64_t", 
        "bool",
		"uint16_t", "uint32_t", "unsigned int", "uint64_t",
		"std::string",
		"glm::vec2", "glm::vec3", "glm::vec4",
		"TransformComponent", "Rigidbody2DComponent",
    };

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

        if (extension.length() > 4)
            return;

        if (strncmp(extension.c_str(), ".cpp", 4) != 0)
            return;

        size_t fileNameLen = fileName.length();

        for (const auto& entry : std::filesystem::recursive_directory_iterator(Project::GetAssetFileSystemPath()))
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

                m_FilesToBePrepared.push_back(entry.path());

            }

        }

    }
    // TODO: When switching to Asset Manager, Only look for .catscript files!
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
                    fileName.erase(fileName.length() - 4); // erases .cpp
                    std::string intermediate = fileName.substr(fileName.length() - 4);
                    if (strncmp(intermediate.c_str(), "-int", 4) != 0)
                        m_FilesToBePrepared.push_back(filePath);
                }

            }

        }
    }

    void SourceFileCompiler::CopyAndPrepareFiles()
    {
        for (auto& path : m_FilesToBePrepared)
        {
            CE_API_WARN(path.string());
            std::ifstream in(path);
            if (!in.is_open())
                CE_API_ASSERT(false, "Failed to open file: {}", path.string().c_str());
    
            std::stringstream ss; ss << in.rdbuf();
            std::string line;

            std::stringstream out;

            bool inClass = false;
            bool firstLoopInClass = true;

            int openingBrackets = 0, closingBrackets = 0;

            std::string className;

            while (std::getline(ss, line))
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
            std::filesystem::path pathCPY = path;
            std::string fileNameStr = path.filename().string();
            std::string filePath = pathCPY.remove_filename();

            size_t extensionPos = fileNameStr.find(".");
            std::string extension = fileNameStr.substr(extensionPos);
            std::string fileName = fileNameStr.substr(0, extensionPos);

            FileDescription fd;
            fd.Name = fileName;
            
            fileName.append("-int");

            fileName.append(extension);

            filePath.append(fileName);

            std::stringstream inCheckSS;
            std::ifstream inCheck(filePath);
            if (inCheck.is_open())
                inCheckSS << inCheck.rdbuf();

            std::ofstream outStream(filePath);
            if (!outStream.is_open())
            {
                std::cout << "Failed to create file!\n";
                return;
            }
            fd.Path = filePath;
            // TODO: STORE THE PROJECT NAME SOMEWHERE
            filePath.erase(0, Project::GetAssetFileSystemPath().string().length() + 1);

            out << "extern \"C\" CatEngine::IScriptObject* create() { return new " << className << "; }\n"
                << "extern \"C\" void destroy(CatEngine::IScriptObject* script) { delete script; }";
            
            outStream << out.str();
            outStream.close();

            fd.FileName = fileName;
            fd.RelativePath = filePath;

            std::filesystem::path compilePath = Project::GetAssetFileSystemPath(".build");
            std::string objName = fd.Name; objName.append("-int.o");
            std::string compileName = "lib"; compileName.append(fd.Name).append("-int.so");

            fd.ObjPath = compilePath / objName;
            fd.CompilePath = compilePath / compileName;


            m_FilesToBeCompiled[fd.Path] = fd;
        }
        m_FilesToBePrepared.clear();

    }

    std::string SourceFileCompiler::GetBuildCommandVariables(const FileDescription& fd)
    {

        std::filesystem::path rootCatEnginePath = Application::Get().GetMainPath();
        std::string CatEngineSRC = rootCatEnginePath.string();
        CatEngineSRC.append("/CatEngine/src");
        std::string CatEngineVND = rootCatEnginePath.string();
        CatEngineVND.append("/vendor");

        std::stringstream ss;
        ss << "gcc -c " << fd.Path.string() << " -o " << fd.ObjPath.string() << " \\\n"
           << "-I" << CatEngineSRC << " \\\n"
           << "-I" << CatEngineVND << "/spdlog/include \\\n"
           << "-I" << CatEngineVND << "/glm \\\n"
           << "-I" << CatEngineVND << "/entt \\\n"
           << "-I" << CatEngineVND << "/Glad/include \\\n"
           << "-DCE_SCRIPT_COMPILATION \\\n"
           << "-fPIC; \n";
     

        ss << "gcc -shared " << fd.ObjPath << " -o " << fd.CompilePath << " -L" << rootCatEnginePath.string() << "/build/CatEngine -lCatEngine -ldl\\\n";
        
        return ss.str();

    }

    void SourceFileCompiler::CompileFiles()
    {
            CE_API_INFO("RECOMPILING!");
            Application::Get().SubmitToMainThread([](){
                for (auto& [path, fd] : m_FilesToBeCompiled)
                {
                    CE_API_INFO("COMILING: {}", fd.Name);
                    std::stringstream ss;
                    ss << "cd " << Project::GetAssetFileSystemPath() << "; " << GetBuildCommandVariables(fd);
                    system(ss.str().c_str());

                    auto it = m_CompiledFiles.find(path);
                    if (it == m_CompiledFiles.end())
                    {
                        m_CompiledFiles.emplace(std::pair<std::filesystem::path, FileDescription>(path, fd));
                    }
                }
                m_FilesToBeCompiled.clear();
            });

    }

    void SourceFileCompiler::Init()
    {
        if (!std::filesystem::exists(Project::GetAssetFileSystemPath(".build")))
        {
            std::filesystem::create_directory(Project::GetAssetFileSystemPath(".build"));
            std::stringstream ss;
            ss << "cd " << Project::GetAssetFileSystemPath(".build") << "; cmake ..; cmake -DCMAKE_BUILD_TYPE=Release .";
            system(ss.str().c_str());
        }
    }

    std::unordered_map<std::filesystem::path, FileDescription> SourceFileCompiler::GetIntermediateFiles()
    {
        return m_CompiledFiles;
    }

}
