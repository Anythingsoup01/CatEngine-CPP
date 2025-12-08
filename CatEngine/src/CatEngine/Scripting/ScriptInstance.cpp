#include "cepch.h"

#include "ScriptInstance.h"
#include "ScriptClass.h"


#include "ScriptEngine.h"


template<>
entt::entity RuntimeValue::As<entt::entity>() const
{
    if (Type != ValueType::INT32) throw std::runtime_error("Type mismatch for int");
    return (entt::entity)ui64;
}


namespace CatEngine
{
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{ 
        m_Instance = scriptClass->Instantiate(entity.GetUUID());
        for (auto& [name, field] :scriptClass->GetFields())
        {
            size_t valueSize = TypeToSize(field.Type);
            std::vector<uint8_t> buffer(valueSize);

            CapyField* cf = scriptClass->m_CapyClass->VTable->Fields[field.Name].get();

            capy_field_data_get(m_Instance, cf, buffer.data());
                

            m_DefaultFieldDatas[field.Name] = std::move(buffer);
        }

        m_StartMethod = scriptClass->GetMethod("Start");
        m_UpdateMethod = scriptClass->GetMethod("Update");

        m_CollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter");
        m_CollisionExitMethod = scriptClass->GetMethod("OnCollisionExit");

	}

    ScriptInstance::~ScriptInstance()
    {
        if (!m_Instance)
            return;

        // Restore default field data safely
        for (auto& [name, buffer] : m_DefaultFieldDatas)
        {
            SetFieldData(name, buffer.data());
        }

        m_DefaultFieldDatas.clear();
    }

	void ScriptInstance::InvokeUpdateMethod(float ts)
	{
        m_ScriptClass->InvokeMethod(m_UpdateMethod, m_Instance, {ts});
	}

	void ScriptInstance::InvokeStartMethod()
	{
        m_ScriptClass->InvokeMethod(m_StartMethod, m_Instance, {});
    }
    void ScriptInstance::InvokeOnCollisionEnter(const UUID& other)
    {
        m_ScriptClass->InvokeMethod(m_CollisionEnterMethod, m_Instance, { other.uuid() });
    }

    void ScriptInstance::InvokeOnCollisionExit(const UUID& other)
    {
        m_ScriptClass->InvokeMethod(m_CollisionExitMethod, m_Instance, { other.uuid() });
    }
	
    bool ScriptInstance::GetFieldDataInternal(const std::string& name, void* buffer)
    {
        std::string nameSpace, className;
        m_ScriptClass->GetNames(nameSpace, className);
        {
        const auto& fields = m_ScriptClass->GetFields();
        UUID fieldHash = ScriptEngine::GetMutable().GetUUIDFromStringHash(nameSpace, className, name);
        auto it = fields.find(fieldHash);
        if (it == fields.end())
            return false;
        }
        
        auto& fields = m_ScriptClass->m_CapyClass->VTable->Fields;
        auto it = fields.find(name);
        if (it == fields.end()) return false;

        CapyField* cf = it->second.get();

        capy_field_data_get(m_Instance, cf, buffer);
        return true;
    }

	void ScriptInstance::SetFieldDataInternal(const std::string& name, void* value)
    {
        int size = 0;
        std::string nameSpace, className;
        m_ScriptClass->GetNames(nameSpace, className);
        {
        const auto& fields = m_ScriptClass->GetFields();
        UUID fieldHash = ScriptEngine::GetMutable().GetUUIDFromStringHash(nameSpace, className, name);
        auto it = fields.find(fieldHash);
        if (it == fields.end())
            return;

        size = TypeToSize(it->second.Type);
        }
        
        auto& fields = m_ScriptClass->m_CapyClass->VTable->Fields;
        auto it = fields.find(name);
        if (it == fields.end()) return;

        CapyField* cf = it->second.get();

        capy_field_data_set(m_Instance, cf, value, size);
    }
}


