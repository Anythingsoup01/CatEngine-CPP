#pragma once

#include "CatScriptCore/Types/Types.h"
#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatEngine
{
    class IScriptObject
    {
    public:
        virtual void Start() {}
        virtual void Update(float ts) {}
        virtual void OnCollisionEnter(const Entity& other) {}
        virtual void OnCollisionExit(const Entity& other) {}

    protected:
        UUID m_UUID;
        IScriptObject* m_Instance;

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

        void ApplyForce(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, const glm::vec2& point, bool wake) { ScriptGlue::Rigidbody2D_ApplyForce(rb2d, &impulse, &point, wake); }
        void ApplyForceToCenter(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, bool wake) { ScriptGlue::Rigidbody2D_ApplyForceToCenter(rb2d, &impulse, wake); }
        void ApplyLinearImpulse(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, const glm::vec2& point, bool wake) { ScriptGlue::Rigidbody2D_ApplyLinearImpulse(rb2d, &impulse, &point, wake); }
        void ApplyLinearImpulseToCenter(Rigidbody2DComponent& rb2d, const glm::vec2& impulse, bool wake) { ScriptGlue::Rigidbody2D_ApplyLinearImpulseToCenter(rb2d, &impulse, wake); }
        
    private:
        void SetUUID(UUID entityID) { m_UUID = entityID; }
    private:
        friend class ScriptInstance;
    };
}
