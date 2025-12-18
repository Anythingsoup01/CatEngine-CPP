#pragma once

#include "ScriptEntityStorage.h"
#include "CatEngine/Core/UUID.h"

namespace CatEngine
{
    class ScriptClass
    {
    public:
        ScriptClass() = default;
        ScriptClass(CapyImage* image, const std::string& nameSpace, const std::string& className);

        CapyObject* Instantiate(UUID entityID);

        CapyMethod* GetMethod(const std::string& methodName);

        void* InvokeMethod(CapyMethod* method, CapyObject* instance, const std::vector<RuntimeValue>& values);

        const std::unordered_map<UUID, ScriptField>& GetFields() const { return m_Fields; }
        std::unordered_map<UUID, ScriptField> m_Fields;
    private:
        std::filesystem::path m_Path;
        CapyClass* m_CapyClass;

        friend class ScriptInstance;
    };
}
