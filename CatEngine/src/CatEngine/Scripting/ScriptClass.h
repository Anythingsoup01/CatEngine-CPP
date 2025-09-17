#pragma once

#include "CatScriptCore.h"

namespace CatEngine
{
    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(const std::filesystem::path& path);

        ~ScriptClass();

        void SetFieldsFromFile(const std::filesystem::path& filePath);

        CatScriptObject* Instantiate() { return m_CreateScript(); }

        void DeleteScript(IScriptObject* scriptObject) { delete scriptObject; }

        const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_Fields; }
    private:
        std::filesystem::path m_Path;
        
        CatScriptClass* m_Instance;

        create_t* m_CreateScript;
        destroy_t* m_DestroyScript;

        std::unordered_map<std::string, ScriptField> m_Fields;

    };
}
