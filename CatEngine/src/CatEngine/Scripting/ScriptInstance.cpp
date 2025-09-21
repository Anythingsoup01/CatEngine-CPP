#include "CatEngine/Core/Core.h"
#include "cepch.h"
#include "ScriptInstance.h"

#include "ScriptClass.h"
#include <cstring>

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
        m_Instance->SetUUID(entityID);
    }

	void ScriptInstance::InvokeUpdateMethod(float ts)
	{
        m_Instance->Update(ts);
	}

	void ScriptInstance::InvokeStartMethod()
	{
        m_Instance->Start();
    }
    void ScriptInstance::InvokeDeleteScript()
    {
        m_ScriptClass->DeleteScript(m_Instance);
    }

	bool ScriptInstance::GetFieldDataInternal(const std::string& name, void* buffer)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return false;

        const ScriptField& field = it->second;
        memcpy(buffer, field.ClassField, sizeof(field.ClassField));
        return true;
    }

	void ScriptInstance::SetFieldDataInternal(const std::string& name, void* value)
    {
        const auto& fields = m_ScriptClass->GetFields();
        auto it = fields.find(name);

        if (it == fields.end())
            return;

        const ScriptField& field = it->second;
        switch (field.Type) 
        {
            case ScriptFieldType::Float: *(float*)field.ClassField = *(float*)value; break;
            case ScriptFieldType::Double: *(double*)field.ClassField = *(double*)value; break;
            case ScriptFieldType::Char: *(char*)field.ClassField = *(char*)value; break;
            case ScriptFieldType::Int16: *(int16_t*)field.ClassField = *(int16_t*)value; break;
            case ScriptFieldType::Int32: *(int32_t*)field.ClassField = *(int32_t*)value; break;
            case ScriptFieldType::Int64: *(int64_t*)field.ClassField = *(int64_t*)value; break;
            case ScriptFieldType::Boolean: *(bool*)field.ClassField = *(bool*)value; break;
            case ScriptFieldType::UInt16: *(uint16_t*)field.ClassField = *(uint16_t*)value; break;
            case ScriptFieldType::UInt32: *(uint32_t*)field.ClassField = *(uint32_t*)value; break;
            case ScriptFieldType::UInt64: *(uint64_t*)field.ClassField = *(uint64_t*)value; break;
            case ScriptFieldType::String: *(std::string*)field.ClassField = *(std::string*)value; break;
            case ScriptFieldType::Vector2: *(glm::vec2*)field.ClassField = *(glm::vec2*)value; break;
            case ScriptFieldType::Vector3: *(glm::vec3*)field.ClassField = *(glm::vec3*)value; break;
            case ScriptFieldType::Vector4: *(glm::vec4*)field.ClassField = *(glm::vec4*)value; break;
            case ScriptFieldType::TransformComponent: *(TransformComponent*)field.ClassField = *(TransformComponent*)value; break;
            case ScriptFieldType::Rigidbody2DComponent: *(Rigidbody2DComponent*)field.ClassField = *(Rigidbody2DComponent*)value; break;
        }
    }
}


