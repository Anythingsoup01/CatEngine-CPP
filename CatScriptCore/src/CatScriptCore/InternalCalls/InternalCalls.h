#pragma once

#include "CatScriptCore/Types/Vector.h"
#include "CatScriptCore/Input/KeyCodes.h"

#include <string>
#include <cstdint>

struct Object;

namespace CatRuntime
{
#   define CE_ADD_INTERNAL_CALL(name, ret, ...) \
        using name##_func_t = ret(*)(__VA_ARGS__); \
        extern name##_func_t name;

#   define CE_INITIALIZE_INTERNAL_CALL(name) \
        name##_func_t name = nullptr;

    CE_ADD_INTERNAL_CALL(Input_IsKeyPressed, bool, KeyCode);
    CE_ADD_INTERNAL_CALL(Input_IsKeyReleased, bool, KeyCode);

    CE_ADD_INTERNAL_CALL(Object_HasComponent, bool, uint64_t, const char*);
    CE_ADD_INTERNAL_CALL(Object_FindObjectByName, uint64_t, const char*);

    CE_ADD_INTERNAL_CALL(Object_GetLayer, const char*, uint64_t);
    CE_ADD_INTERNAL_CALL(Object_SetLayer, void, uint64_t, const char*);

    CE_ADD_INTERNAL_CALL(Object_GetTag, const char*, uint64_t);
    CE_ADD_INTERNAL_CALL(Object_SetTag, void, uint64_t, const char*);

    CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForce, void, uint64_t, Vector2*, Vector2*, bool);
    CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForceToCenter, void, uint64_t, Vector2*, bool);
    CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulse, void, uint64_t, Vector2*, Vector2*, bool);
    CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulseToCenter, void, uint64_t, Vector2*, bool);

    CE_ADD_INTERNAL_CALL(Transform_GetPosition, void, uint64_t, Vector3*);
    CE_ADD_INTERNAL_CALL(Transform_SetPosition, void, uint64_t, Vector3*);
    CE_ADD_INTERNAL_CALL(Transform_GetRotation, void, uint64_t, Vector3*);
    CE_ADD_INTERNAL_CALL(Transform_SetRotation, void, uint64_t, Vector3*);
    CE_ADD_INTERNAL_CALL(Transform_GetScale, void, uint64_t, Vector3*);
    CE_ADD_INTERNAL_CALL(Transform_SetScale, void, uint64_t, Vector3*);

    CE_ADD_INTERNAL_CALL(Sprite_GetColor, void, uint64_t, Vector4*);
    CE_ADD_INTERNAL_CALL(Sprite_SetColor, void, uint64_t, Vector4*);
    CE_ADD_INTERNAL_CALL(Sprite_GetTextureID, void, uint64_t, uint64_t*);
    CE_ADD_INTERNAL_CALL(Sprite_SetTextureID, void, uint64_t, uint64_t*);
}
