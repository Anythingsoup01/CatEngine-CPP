#include "InternalCalls.h"

namespace CatRuntime
{
    CE_INITIALIZE_INTERNAL_CALL(Input_IsKeyDown);

    CE_INITIALIZE_INTERNAL_CALL(Object_HasComponent);
    CE_INITIALIZE_INTERNAL_CALL(Object_FindObjectByName);

    CE_INITIALIZE_INTERNAL_CALL(GetScriptInstance);

    CE_INITIALIZE_INTERNAL_CALL(Rigidbody2D_ApplyForce);
    CE_INITIALIZE_INTERNAL_CALL(Rigidbody2D_ApplyForceToCenter);
    CE_INITIALIZE_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulse);
    CE_INITIALIZE_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulseToCenter);

    CE_INITIALIZE_INTERNAL_CALL(Transform_GetPosition);
    CE_INITIALIZE_INTERNAL_CALL(Transform_SetPosition);
    CE_INITIALIZE_INTERNAL_CALL(Transform_GetRotation);
    CE_INITIALIZE_INTERNAL_CALL(Transform_SetRotation);
    CE_INITIALIZE_INTERNAL_CALL(Transform_GetScale);
    CE_INITIALIZE_INTERNAL_CALL(Transform_SetScale);

}
