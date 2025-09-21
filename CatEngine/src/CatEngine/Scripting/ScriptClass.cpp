#include "CatEngine/Scripting/CatScriptCore.h"
#include "cepch.h"
#include "ScriptClass.h"

#include "ScriptInstance.h"
#include <cstdint>
#include <cstring>

#ifdef CE_PLATFORM_LINUX
#include <dlfcn.h>
#endif

namespace CatEngine
{

    static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{"float", ScriptFieldType::Float},
		{"double", ScriptFieldType::Double},
		{"char",ScriptFieldType::Char},
		{"int16_t", ScriptFieldType::Int16},
		{"int32_t", ScriptFieldType::Int32},
		{"int", ScriptFieldType::Int32},
		{"int64_t", ScriptFieldType::Int64},
		{"bool", ScriptFieldType::Boolean},
		{"uint16_t", ScriptFieldType::UInt16},
		{"uint32_t", ScriptFieldType::UInt32},
		{"unsigned int", ScriptFieldType::UInt32},
		{"uint64_t", ScriptFieldType::UInt64},
		{"std::string", ScriptFieldType::String},
		{"glm::vec2", ScriptFieldType::Vector2},
		{"glm::vec3", ScriptFieldType::Vector3},
		{"glm::vec4", ScriptFieldType::Vector4},
		{"TransformComponent", ScriptFieldType::TransformComponent},
		{"Rigidbody2DComponent", ScriptFieldType::Rigidbody2DComponent},

	};

    static bool IsVariable(const char* line, std::string& outVariableType, std::string& outVariableName, void* outVariableValue)
    {
        for (auto& [keyword, sft] : s_ScriptFieldTypeMap)
        {
            if (strncmp(line, keyword.c_str(), keyword.length()) == 0)
            {
                std::string lineStr(line);
                outVariableType = keyword;
                lineStr.erase(0, keyword.length());
                size_t equalSign = lineStr.find_first_of('=');
                if (equalSign == std::string::npos)
                    outVariableName = lineStr.substr(0 , lineStr.length() - 1); // Erases ";" character
                else
                    outVariableName = lineStr.substr(0, equalSign);
                
                return true;
            }
        }

        return false;
    }

    ScriptFieldType StringToScriptFieldType(const std::string& type)
    {
        auto it = s_ScriptFieldTypeMap.find(type);

        if (it != s_ScriptFieldTypeMap.end())
            return it->second;

        CE_API_ERROR("Not supported type {}", type);
        return ScriptFieldType::None;
    }

    void* GetVariableSymbol(CatScriptClass* handle, ScriptFieldType type, const std::string& symbolName)
    {
        void* out = nullptr;

        switch (type)
        {
#ifdef      CE_PLATFORM_LINUX
            case ScriptFieldType::Float: out = (float*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Double: out = (double*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Char: out = (char**)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int16: out = (int16_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int32: out = (int32_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Int64: out = (int64_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Boolean: out = (bool*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt16: out = (uint16_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt32: out = (uint32_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::UInt64: out = (uint64_t*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::String: out = (std::string*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector2: out = (glm::vec2*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector3: out = (glm::vec3*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Vector4: out = (glm::vec4*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::TransformComponent: out = (TransformComponent*)dlsym(handle, symbolName.c_str()); break;
            case ScriptFieldType::Rigidbody2DComponent: out = (Rigidbody2DComponent*)dlsym(handle, symbolName.c_str()); break;
#           endif
            default: break;
        }

        return out;

    }

    bool IsTrue(const std::string& value)
    {
        if (strncmp(value.c_str(), "true", 4) == 0)
            return true;
        return false;
    }

    ScriptField GetField(CatScriptClass* instance, const char* line, std::string& outName)
    {
        ScriptField sf;
        std::string type, value;

        std::string lineStr(line);

        for (auto& [keyword, sft] : s_ScriptFieldTypeMap)
        {
            if (strncmp(line, keyword.c_str(), keyword.length()) == 0)
            {
                sf.Type = sft;
                type = keyword;
                lineStr.erase(0, keyword.length());
                size_t equalSign = lineStr.find_first_of('=');
                if (equalSign != std::string::npos)
                    outName = lineStr.substr(0, equalSign);
                else
                    outName = lineStr.substr(0 , lineStr.length() - 1); // Erases ";" character

                outName.erase(remove_if(outName.begin(), outName.end(), isspace), outName.end()); // Removes and folling and trailing spaces

                sf.ClassField = GetVariableSymbol(instance, sft, outName);
                if (sf.ClassField == nullptr)
                {
                    CE_API_CRITICAL("NULL DETECTED: {0}:{1}", type, outName);
                    outName = "";
                    return ScriptField();
                }
                break;
            }
        }
        return sf;

    }

    ScriptClass::ScriptClass(const std::filesystem::path& path)
        : m_Path(path)
    {
#       ifdef CE_PLATFORM_LINUX
        m_Instance = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        CE_API_ASSERT(m_Instance, dlerror());

        m_CreateScript = (create_t*)dlsym(m_Instance, "create");
        CE_API_ASSERT(m_CreateScript, dlerror());
        m_DestroyScript = (destroy_t*)dlsym(m_Instance, "destroy");
        CE_API_ASSERT(m_DestroyScript, dlerror());

#       endif
    }

    ScriptClass::~ScriptClass()
    {
#       ifdef CE_PLATFORM_LINUX
        dlclose(m_Instance);
#       endif 
        m_Instance = nullptr;
    }

    void ScriptClass::SetFieldsFromFile(const std::filesystem::path& filePath)
    {
    
        std::ifstream in(filePath);
        if (!in.is_open())
        {
            CE_API_ERROR("Failed to open file '{}'", filePath.string());
            return;
        }

        std::string line;

        bool isPublicVariable = false;
        bool inClass = false;
        bool firstLoopInClass = true;

        int openingBrackets = 0, closingBrackets = 0;

        std::unordered_map<std::string, std::string> variables;

        while (std::getline(in, line))
        {
            std::string lineParse = line;
            lineParse.erase(remove_if(lineParse.begin(), lineParse.end(), isspace), lineParse.end());

            if (strncmp(lineParse.c_str(), "#definePUBLIC", 13) == 0)
                isPublicVariable = true;

            if (strncmp(lineParse.c_str(), "#definePRIVATE", 14) == 0)
                isPublicVariable = false;


            if (strncmp(lineParse.c_str(), "class", 5) == 0)
            {
                inClass = true;
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
                if (isPublicVariable)
                {
                    std::string name;
                    ScriptField sf = GetField(m_Instance, line.c_str(), name);
                    if (!name.empty())
                    {
                        m_Fields[name] = sf;
                    }
                }
            }

        }
    
    }
}
