#pragma once

#include "CatEngine/Scene/Entity.h"
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

void DrawCanvasComponent(Entity selection);
void DrawCanvasGroupComponent(Entity selection);
void DrawPanelComponent(Entity selection);
void DrawButtonComponent(Entity selection);
void DrawRectTransformComponent(Entity selection);
void DrawSelectableComponent(Entity selection);
void DrawEventListenerComponent(Entity selection);

}
