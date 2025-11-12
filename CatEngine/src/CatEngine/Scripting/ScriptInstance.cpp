#include "CatEngine/Core/Core.h"
#include "CatEngine/Scene/Entity.h"
#include "cepch.h"
#include "ScriptInstance.h"

#include "ScriptClass.h"
#include <cstring>


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
        m_Instance = scriptClass->Instantiate();
        for (auto& [name, field] :scriptClass->GetFields())
        {
            size_t valueSize = TypeToSize(field.Type);
            void* copy = malloc(valueSize);
            if (copy == nullptr)
            {
                CE_API_ASSERT(false, "Failed to malloc data! : NAME - {}", name);
            }

            memcpy(copy, field.ClassField, valueSize);

            if(copy == nullptr)
            {
                CE_API_ASSERT(false, "Failed to memcpy data! : NAME - {}", name);
            }

            m_DefaultFieldDatas[name] = copy;
        }

        m_StartMethod = scriptClass->GetMethod("Start");
        m_UpdateMethod = scriptClass->GetMethod("Update");

        m_CollisionEnterMethod = scriptClass->GetMethod("OnCollisionEnter");
        m_CollisionExitMethod = scriptClass->GetMethod("OnCollisionExit");

        m_SetUUIDMethod = scriptClass->GetMethod("SetInstanceID");

	}

    ScriptInstance::~ScriptInstance()
    {
        for (auto& [name, data] : m_DefaultFieldDatas)
        {
            SetFieldDataInternal(name, data);
            free(data);
        }

        m_DefaultFieldDatas.clear();
    }

    void ScriptInstance::SetEntityID(UUID entityID)
    {
        if (!m_SetUUIDMethod)
            CE_API_CRITICAL("CAN'T ACCESS SETUUID!");
        else
            m_ScriptClass->InvokeMethod(m_SetUUIDMethod, m_Instance, { (int)entityID });
    }

	void ScriptInstance::InvokeUpdateMethod(float ts)
	{
        m_ScriptClass->InvokeMethod(m_UpdateMethod, m_Instance, {ts});
	}

	void ScriptInstance::InvokeStartMethod()
	{
        m_ScriptClass->InvokeMethod(m_StartMethod, m_Instance, {});
    }
    void ScriptInstance::InvokeOnCollisionEnter(const Entity& other)
    {
        m_ScriptClass->InvokeMethod(m_CollisionEnterMethod, m_Instance, { (int)other });
    }

    void ScriptInstance::InvokeOnCollisionExit(const Entity& other)
    {
        m_ScriptClass->InvokeMethod(m_CollisionExitMethod, m_Instance, { (int)other });
    }
	
    bool ScriptInstance::GetFieldDataInternal(const std::string& name, void* buffer)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return false;

        ScriptField field = it->second;
        memcpy(buffer, field.ClassField->SymHandle, TypeToSize(field.Type));
        return true;
    }

	void ScriptInstance::SetFieldDataInternal(const std::string& name, void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return;

        ScriptField field = it->second;

        memcpy(field.ClassField->SymHandle, value, TypeToSize(field.Type));
    }
}


