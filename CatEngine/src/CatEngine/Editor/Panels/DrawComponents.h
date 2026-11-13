#pragma once

#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"
namespace CatEngine {

void DrawComponentsHelper(Entity selection);

void DrawTransformComponent(Entity selection);
void DrawSpriteRendererComponent(Entity selection);
void DrawCircleRendererComponent(Entity selection);
void DrawCameraComponent(Entity selection);
void DrawRigidbody2DComponent(Entity selection);
void DrawBoxCollider2DComponent(Entity selection);
void DrawCircleCollider2DComponent(Entity selection);
void DrawScriptComponent(Entity selection);

}
