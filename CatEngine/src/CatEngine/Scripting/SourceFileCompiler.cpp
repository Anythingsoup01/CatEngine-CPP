
#include "cepch.h"
#include "SourceFileCompiler.h"

#include "CatEngine/Core/Application.h"
#include <sstream>
#include <vector>

namespace CatEngine
{
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
    
    void SourceFileCompiler::AddFile(const std::string& filePath)
    {
        size_t strLen = filePath.length();
        std::string extension = filePath.substr(strLen - 4);

        if (strncmp(extension.c_str(), ".cpp", 4) == 0)
        {
            m_FilesToBePrepared.push_back(filePath);
            CopyAndPrepareFiles();
            CompileFiles();
        }
    }

    void SourceFileCompiler::AddDirectory(const std::filesystem::path& directory) 
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) 
        {
            std::string path = entry.path().string();
            size_t hidden = path.find(".build");
            if (entry.is_regular_file() && entry.path().extension() == ".cpp" && (hidden == std::string::npos)) 
            {
                m_FilesToBePrepared.push_back(entry.path());
            }
        }
    }

    void SourceFileCompiler::RemoveFile(const std::filesystem::path &filePath)
    {
        m_FilesToBePrepared.erase(std::remove(m_FilesToBePrepared.begin(), m_FilesToBePrepared.end(), filePath), m_FilesToBePrepared.end());
        auto it = m_FilesToBeCompiled.find(filePath);
        if (it != m_FilesToBeCompiled.end())
            m_FilesToBeCompiled.erase(it);
    }
    
    void SourceFileCompiler::CopyAndPrepareFiles()
    {
        if (m_Preparing)
            return;

        m_Preparing = true;
        for (auto& path : m_FilesToBePrepared)
        {
            {
                if (path.extension() != ".cpp")
                    continue;

                std::filesystem::path pf = path;
                std::string fileNameStr = pf.filename();
                if (fileNameStr.find("-int") != std::string::npos)
                    continue;

                std::filesystem::file_time_type lastModified = std::filesystem::last_write_time(path);
                std::optional<std::filesystem::file_time_type> lastCompiled;

                if (m_CompiledFiles.find(path) != m_CompiledFiles.end())
                    lastCompiled = std::filesystem::last_write_time(m_CompiledFiles[path].Path);

                if (lastModified <= lastCompiled)
                    continue;
            }
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
                        ss << "extern \"C\" " << line;
                        line = ss.str();
                    }

                }
                out << line << "\n";
            }

            std::string filePath = path.relative_path().remove_filename();
            std::string fileNameStr = path.filename().string();

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
            filePath.erase(0, 21); // sizeof("SampleProject/Assets/")

            out << "extern \"C\" CatEngine::IScriptObject* create() { return new " << className << "; }\n"
                << "extern \"C\" void destroy(CatEngine::IScriptObject* script) { delete script; }";
            
            outStream << out.str();
            outStream.close();

            if (out.str() != inCheckSS.str() || m_NeedsRecompiled)
            {
                m_NeedsRecompiled = true;
            }

            fd.FileName = fileName;
            fd.RelativePath = filePath;
            std::string projectPath = "SampleProject/";
            std::string compilePath = projectPath;
            compilePath.append("Assets/.build/lib").append(fd.Name).append(".so");

            fd.CompilePath = compilePath;

            m_FilesToBeCompiled[fd.Path] = fd;
        }
        m_Preparing = false;

        RegenerateCmakeFile();
    }

    void SourceFileCompiler::RegenerateCmakeFile()
    {
        std::stringstream inSS;
        std::ifstream in("SampleProject/Assets/CMakeLists.txt");
        if (in.is_open())
            inSS << in.rdbuf();
        
        std::ofstream out("SampleProject/Assets/CMakeLists.txt");
        if (!out.is_open())
            CE_API_ASSERT(false, "Failed to generate file!");

        std::filesystem::path rootCatEnginePath = Application::Get().GetMainPath();
        std::string CatEngineSRC = rootCatEnginePath.string();
        CatEngineSRC.append("/CatEngine/src");
        std::string CatEngineVND = rootCatEnginePath.string();
        CatEngineVND.append("/vendor");

        std::stringstream ss;
        ss << "cmake_minimum_required(VERSION 3.15)\n"
           << "project(" << "SampleProject" << ")\n"
           << "set(INCLUDEDIRS\n"
           << "    " << CatEngineSRC << "\n"
           << "    " << CatEngineVND << "/spdlog/include\n"
           << "    " << CatEngineVND << "/glm\n"
           << "    " << CatEngineVND << "/entt\n"
           << "    " << CatEngineVND << "/Glad/include)\n\n"
           << "set(PRECOMPILEDHEADER " << CatEngineSRC << "/cepch.h)\n"
           << "set(CATENGINELIB " << rootCatEnginePath.string() << "/build/CatEngine/libCatEngine.a dl)\n"
           << "set(COMPILEFLAGS CE_SCRIPT_COMPILATION)\n"
           << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -rdynamic -fPIC\")  \n";

        for (auto& [path, fd] : m_FilesToBeCompiled)
        {
            ss << "add_library(" << fd.Name << " SHARED " << fd.RelativePath << ")\n"
               << "target_include_directories(" << fd.Name << " PRIVATE ${INCLUDEDIRS})\n"
               << "target_precompile_headers(" << fd.Name << " PRIVATE ${PRECOMPILEDHEADER})\n"
               << "target_link_libraries(" << fd.Name << " PRIVATE ${CATENGINELIB})\n"
               << "target_compile_definitions(" << fd.Name << " PRIVATE ${COMPILEFLAGS})\n\n";
        }
        
        out << ss.str();

        out.close();
    }

    void SourceFileCompiler::CompileFiles()
    {
        if (m_NeedsRecompiled)
        {
            Application::Get().SubmitToMainThread([](){
                system("cd SampleProject/Assets && cmake --build .build/ -j 10");
            });
            m_CompiledFiles = m_FilesToBeCompiled;
            m_FilesToBeCompiled.clear();
            m_NeedsRecompiled = false;
        }
    }

    void SourceFileCompiler::Init()
    {
        if (!std::filesystem::exists("SampleProject/Assets/.build"))
        {
            std::filesystem::create_directory("SampleProject/Assets/.build");
            system("cd SampleProject/Assets/.build; cmake ..; cmake -DCMAKE_BUILD_TYPE=Release .");
        }
    }

    std::unordered_map<std::filesystem::path, FileDescription> SourceFileCompiler::GetIntermediateFiles()
    {
        return m_CompiledFiles;
    }

}
