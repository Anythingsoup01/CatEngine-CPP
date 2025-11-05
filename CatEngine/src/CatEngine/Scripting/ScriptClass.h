#pragma once

#include "CatScriptCore.h"

namespace CatEngine
{
    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(CapyImage* image, const std::string& nameSpace, const std::string& className);

        void* Instantiate();

        CapyMethod* GetMethod(const std::string& methodName);

        void* InvokeMethod(CapyMethod* method, void* instance, const std::vector<RuntimeValue>& values);

        const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_Fields; }
    private:
        std::filesystem::path m_Path;
        
        CapyClass* m_CapyClass;

        std::unordered_map<std::string, ScriptField> m_Fields;

    };
}
