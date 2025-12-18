#include "cepch.h"

#include "ScriptInstance.h"
#include "ScriptClass.h"


#include "ScriptEngine.h"



namespace CatEngine
{
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{ 
        m_Instance = scriptClass->Instantiate(entity.GetUUID());
        for (auto& [fieldID, field] :scriptClass->GetFields())
        {
            size_t valueSize = TypeToSize(field.Type);
            std::vector<uint8_t> buffer(valueSize);

            CapyField* cf = scriptClass->m_CapyClass->VTable->Fields[fieldID].get();

            capy_field_data_get(m_Instance, cf, buffer.data());
                

            m_DefaultFieldDatas[field.Name] = std::move(buffer);
        }

        uint64_t data = entity.GetUUID();
        SetFieldData("m_EntityID", data);

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
        if (m_UpdateMethod)
            m_ScriptClass->InvokeMethod(m_UpdateMethod, m_Instance, {ts});
	}

	void ScriptInstance::InvokeStartMethod()
	{
        if (m_StartMethod)
            m_ScriptClass->InvokeMethod(m_StartMethod, m_Instance, {});
    }
    void ScriptInstance::InvokeOnCollisionEnter(const UUID& other)
    {
        if (m_CollisionEnterMethod)
            m_ScriptClass->InvokeMethod(m_CollisionEnterMethod, m_Instance, { other.uuid() });
    }

    void ScriptInstance::InvokeOnCollisionExit(const UUID& other)
    {
        if (m_CollisionExitMethod)
            m_ScriptClass->InvokeMethod(m_CollisionExitMethod, m_Instance, { other.uuid() });
    }
	
    bool ScriptInstance::GetFieldDataInternal(const std::string& name, void* buffer)
    {
        std::string nameSpace, className;
        
        const auto& classFields = m_ScriptClass->GetFields();

        bool found = false;
        for (auto& [uuid, scriptField] : classFields)
        {
            if (name == scriptField.Name)
            {
                nameSpace = scriptField.NameSpace;
                className = scriptField.ClassName;
                found = true;
                break;
            }
        }

        if (!found)
            return false;
 
        std::string fullName;
        if (!nameSpace.empty() && !className.empty())
            fullName = nameSpace + "::" + className;
        else if (!nameSpace.empty())
            fullName = nameSpace;
        else if (!className.empty())
            fullName = className;

        fullName += "::" + name;


        uint32_t fieldID = generate_hash(fullName);
        auto& fields = m_ScriptClass->m_CapyClass->VTable->Fields;
        auto it = fields.find(fieldID);
        if (it == fields.end()) return false;

        CapyField* cf = it->second.get();

        capy_field_data_get(m_Instance, cf, buffer);
        return true;
    }

	void ScriptInstance::SetFieldDataInternal(const std::string& name, void* value)
    {
        int size = 0;
        std::string nameSpace, className;
        
        const auto& classFields = m_ScriptClass->GetFields();

        bool found = false;
        for (auto& [uuid, scriptField] : classFields)
        {
            if (name == scriptField.Name)
            {
                nameSpace = scriptField.NameSpace;
                className = scriptField.ClassName;
                found = true;
                break;
            }
        }

        if (!found)
            return;
 
        std::string fullName;
        if (!nameSpace.empty() && !className.empty())
            fullName = nameSpace + "::" + className;
        else if (!nameSpace.empty())
            fullName = nameSpace;
        else if (!className.empty())
            fullName = className;

        fullName += "::" + name;


        uint32_t fieldID = generate_hash(fullName);
        auto& fields = m_ScriptClass->m_CapyClass->VTable->Fields;
        auto it = fields.find(fieldID);
        if (it == fields.end())
            return;

        CapyField* cf = it->second.get();
        capy_field_data_set(m_Instance, cf, value);
    }
}
