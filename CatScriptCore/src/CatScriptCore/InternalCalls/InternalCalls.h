#pragma once

#include "CatScriptCore/Input/KeyCodes.h"
#include "CatScriptCore/Types/Vector.h"

#include <cstdint>
#include <ostream>

#ifdef __cplusplus
extern "C" {
#endif

#define CE_ADD_INTERNAL_CALL(name, ret, ...) \
    using name##_func_t = ret(*)(__VA_ARGS__); \
    extern __attribute__((visibility("default"))) name##_func_t name;

#define CE_INITIALIZE_INTERNAL_CALL(name) \
    __attribute__((visibility("default"))) name##_func_t name = nullptr;

CE_ADD_INTERNAL_CALL(Input_IsKeyPressed, bool, CatRuntime::KeyCode);
CE_ADD_INTERNAL_CALL(Input_IsKeyReleased, bool, CatRuntime::KeyCode);

CE_ADD_INTERNAL_CALL(Object_HasComponent, bool, uint64_t, const char *);
CE_ADD_INTERNAL_CALL(Object_FindObjectByName, uint64_t, const char *);

CE_ADD_INTERNAL_CALL(Object_GetLayer, const char *, uint64_t);
CE_ADD_INTERNAL_CALL(Object_SetLayer, void, uint64_t, const char *);

CE_ADD_INTERNAL_CALL(Object_GetTag, const char *, uint64_t);
CE_ADD_INTERNAL_CALL(Object_SetTag, void, uint64_t, const char *);

CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForce, void, uint64_t, CatRuntime::Vector2 *,
                     CatRuntime::Vector2 *, bool);
CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForceToCenter, void, uint64_t, CatRuntime::Vector2 *,
                     bool);
CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulse, void, uint64_t, CatRuntime::Vector2 *,
                     CatRuntime::Vector2 *, bool);
CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulseToCenter, void, uint64_t,
                     CatRuntime::Vector2 *, bool);

CE_ADD_INTERNAL_CALL(Transform_GetPosition, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetPosition, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_GetRotation, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetRotation, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_GetScale, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetScale, void, uint64_t, CatRuntime::Vector3 *);

CE_ADD_INTERNAL_CALL(Transform_GetLocalPosition, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetLocalPosition, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_GetLocalRotation, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetLocalRotation, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_GetLocalScale, void, uint64_t, CatRuntime::Vector3 *);
CE_ADD_INTERNAL_CALL(Transform_SetLocalScale, void, uint64_t, CatRuntime::Vector3 *);


CE_ADD_INTERNAL_CALL(Sprite_GetColor, void, uint64_t, CatRuntime::Vector4 *);
CE_ADD_INTERNAL_CALL(Sprite_SetColor, void, uint64_t, CatRuntime::Vector4 *);
CE_ADD_INTERNAL_CALL(Sprite_GetTextureID, void, uint64_t, uint64_t *);
CE_ADD_INTERNAL_CALL(Sprite_SetTextureID, void, uint64_t, uint64_t *);

// Declare the internal call

#ifdef __cplusplus
}
#endif

