
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

        template<typename T>
        T GetComponent() { return ScriptGlue::Object_GetComponent<T>(m_UUID); }
        template<typename T>
        void HasComponent() { ScriptGlue::Object_HasComponent(m_UUID, T()); }

        void ApplyForce(Rigidbody2DComponent& rb2d, glm::vec2& impulse, glm::vec2& point, bool wake) { ScriptGlue::Rigidbody2D_ApplyForce(rb2d, &impulse, &point, wake); }

    private:
        void SetUUID(UUID entityID) { m_UUID = entityID; }

    private:
        friend class ScriptInstance;
    };

    typedef IScriptObject* create_t();
    typedef void destroy_t(IScriptObject*);
}
