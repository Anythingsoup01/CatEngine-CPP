#pragma once

#include "CatEngine/Core/Input.h"
#include "CatEngine/Core/KeyCodes.h"
#include "CatEngine/Core/MouseCodes.h"
#include "CatEngine/Math/Math.h"
#include "CatEngine/Scene/Scene.h"
#include "CatEngine/Scene/Entity.h"
namespace CatEngine
{
    class ScriptGlue
    {
    public:
        static Entity GetEntity(UUID entityID);
        static bool Input_IsKeyDown(KeyCode keyCode);

        template<typename T>
        static bool Object_HasComponent(UUID entityID, T component);
        template<typename T>
        static T Object_GetComponent(UUID entityID);


        static uint64_t Object_FindObjectByName(const std::string& name);
	
        static void* GetScriptInstance(UUID entityID);

        static void Rigidbody2D_ApplyForce(Rigidbody2DComponent& rb2d, glm::vec2* impulse, glm::vec2* point, bool wake);
        static void Rigidbody2D_ApplyForceToCenter(UUID entityID, glm::vec2* impulse, bool wake);
        static void Rigidbody2D_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake);
        static void Rigidbody2D_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake);


        static glm::vec3 Transform_GetPosition(UUID entityID);
        static void Transform_SetPosition(UUID entityID, const glm::vec3& position);
        static glm::vec3 Transform_GetRotation(UUID entityID);
        static void Transform_SetRotation(UUID entityID, const glm::vec3& rotation);
        static glm::vec3 Transform_GetScale(UUID entityID);
        static void Transform_SetScale(UUID entityID, const glm::vec3& scale);

        static void RegisterComponents();
    };
}
