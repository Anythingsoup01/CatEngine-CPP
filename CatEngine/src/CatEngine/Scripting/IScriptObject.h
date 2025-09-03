
#pragma once

#include "CatEngine/Core/KeyCodes.h"
#ifndef CE_SCRIPT_COMPILATION
#include <glm/glm.hpp>
#include "ScriptGlue.h"
#include "CatEngine/Core/UUID.h"
#endif

namespace CatEngine
{
    class IScriptObject
    {
    public:
        virtual void Start() {};
        virtual void Update(float ts) {};
        virtual ~IScriptObject() {}
    protected:
        UUID m_UUID;

        bool KeyPressed(KeyCode keyCode) { return ScriptGlue::Input_IsKeyDown(keyCode); }

        void SetPosition(const glm::vec3& pos) { ScriptGlue::Transform_SetPosition(m_UUID, pos); }
        glm::vec3 GetPosition() { return ScriptGlue::Transform_GetPosition(m_UUID); }

        void SetRotation(const glm::vec3& rot) { ScriptGlue::Transform_SetRotation(m_UUID, rot); }
        glm::vec3 GetRotation() { return ScriptGlue::Transform_GetScale(m_UUID); }

        void SetScale(const glm::vec3& scale) { ScriptGlue::Transform_SetScale(m_UUID, scale); }
        glm::vec3 GetScale() { return ScriptGlue::Transform_GetScale(m_UUID); }
    private:
        void SetUUID(UUID entityID) { m_UUID = entityID; }

    private:
        friend class ScriptInstance;
    };

    typedef IScriptObject* create_t();
    typedef void destroy_t(IScriptObject*);
}
