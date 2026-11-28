#pragma once

#include "2D/CircleRenderer.h"
#include "2D/SpriteRenderer.h"

#include "3D/TransformComponent.h"

#include "Asset/PrefabLink.h"

#include "Renderer/CameraComponent.h"

#include "Physics/BoxCollider2D.h"
#include "Physics/CircleCollider2D.h"
#include "Physics/Rigidbody2D.h"

#include "Required/InformationComponents.h"
#include "Required/RelationshipComponent.h" // IWYU pragma: keep

#include "Scripting/ScriptComponentContainer.h"

#include "UI/ButtonComponent.h"
#include "UI/CanvasComponent.h"
#include "UI/CanvasGroupComponent.h"
#include "UI/EventListenerComponent.h"
#include "UI/PanelComponent.h"
#include "UI/RectTransformComponent.h"
#include "UI/SelectableComponent.h"

namespace CatEngine {

template <typename... Component> struct ComponentGroup {};

using AllComponents = ComponentGroup<
    TagComponent, LayerComponent, ObjectStateComponent, TransformComponent,
    CameraComponent, SpriteRendererComponent, CircleRendererComponent,
    BoxCollider2DComponent, CircleCollider2DComponent, Rigidbody2DComponent,
    ScriptComponentContainer, PrefabLinkComponent, RectTransformComponent,
    ButtonVisualComponent, PanelComponent, CanvasComponent, CanvasGroupComponent,
    EventListenerComponent, SelectableComponent>;

template <typename T> const char *ComponentToString() { return "NULL"; }

template <> const char *ComponentToString<TagComponent>();

} // namespace CatEngine
