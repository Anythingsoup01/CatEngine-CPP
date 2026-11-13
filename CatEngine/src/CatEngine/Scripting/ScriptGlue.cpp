#include "ScriptGlue.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
#include "cepch.h"

#include "CatEngine/Core/Input.h"
#include "CatEngine/Core/KeyCodes.h"
#include "CatEngine/Core/MouseCodes.h"
#include "CatEngine/Math/Math.h"
#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scene/Scene.h"
#include "ScriptEngine.h"

#include "CatEngine/Physics/Physics2D.h"

#include "CatEngine/Scene/Components/Components.h"
#include <cstdint>
#include <cstring>

#include <cxxabi.h>

namespace CatEngine {

static std::unordered_map<std::string, std::function<bool(Entity)>>
    s_EntityHasComponentFuncs;

#define CE_ADD_INTERNAL_CALL(Name)                                             \
  Capybara::Capybara::AddInternalCall(#Name, (void *)&Name)

static Entity GetEntity(UUID entityID) {
  Ref<Scene> scene = ScriptEngine::GetMutable().GetSceneContext();
  CE_ASSERT(scene);
  Entity entity = scene->GetEntityByUUID(entityID);
  CE_ASSERT(entity);
  return entity;
}

static bool Input_IsKeyPressed(KeyCode keyCode) {
  return Input::IsKeyPressed(keyCode);
}

static bool Input_IsKeyReleased(KeyCode keyCode) {
  return Input::IsKeyReleased(keyCode);
}

#pragma region Object

static bool Object_HasComponent(UUID entityID, const char *componentType) {
  const Entity &entity = GetEntity(entityID);
  CE_API_ASSERT(s_EntityHasComponentFuncs.find(componentType) !=
                    s_EntityHasComponentFuncs.end(),
                "Component not found: {}", componentType);
  return s_EntityHasComponentFuncs.at(componentType)(entity);
}

std::string GetCleanClassName(const std::string& mangledName) {
  int status = 0;
  // Demangle the symbol using the Itanium C++ ABI
  char* demangled = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);

  if (status == 0 && demangled) {
    std::string fullName(demangled);
    std::free(demangled); // __cxa_demangle allocates memory with malloc, must be freed

    // Find the last scope resolution operator "::" to strip namespaces
    size_t lastColon = fullName.rfind("::");
    if (lastColon != std::string::npos) {
      return fullName.substr(lastColon + 2); // Returns "PlayerController"
    }
    return fullName;
  }

  // Fallback if demangling fails
  return mangledName;
}

static bool Object_HasScript(UUID entityID, const char *scriptName) {
  const Entity &entity = GetEntity(entityID);
  if (!entity.HasComponent<ScriptComponentContainer>())
    return false;

  auto &scc = entity.GetComponent<ScriptComponentContainer>();

  std::string demangledClassName = GetCleanClassName(scriptName);
  for (auto &script : scc.Scripts) {
    if (script.ClassName == demangledClassName) {
      return true;
    }
  }
  return false;
}

static void *Object_GetScript(UUID entityID, const char *scriptName) {
  const Entity &entity = GetEntity(entityID);
  if (!entity.HasComponent<ScriptComponentContainer>())
    return nullptr;

  auto &scc = entity.GetComponent<ScriptComponentContainer>();
  std::string demangledClassName = GetCleanClassName(scriptName);
  for (auto &script : scc.Scripts) {
    if (script.ClassName == demangledClassName) {
      auto scriptInstance = ScriptEngine::GetMutable().GetEntityScriptInstance(entityID, script.ScriptID);

      if (!scriptInstance || !scriptInstance->GetManagedObject())
        return nullptr;

      return scriptInstance->GetManagedObject()->RawMemory.data();
    }
  }
  return nullptr;
}

static uint64_t Object_FindObjectByName(const std::string &name) {
  Ref<Scene> scene = ScriptEngine::GetMutable().GetSceneContext();
  CE_ASSERT(scene);
  Entity entity = scene->FindEntityByName(name.c_str());
  CE_ASSERT(entity);

  if (!entity)
    return 0;

  return entity.GetUUID();
}

static const char *Object_GetLayer(UUID entityID) {
  const Entity &entity = GetEntity(entityID);

  if (!entity.HasComponent<LayerComponent>())
    return "ANON";

  return entity.GetComponent<LayerComponent>().Layer.c_str();
}

static void Object_SetLayer(UUID entityID, const char *value) {
  const Entity &entity = GetEntity(entityID);

  if (!entity.HasComponent<LayerComponent>())
    return;

  entity.GetComponent<LayerComponent>().Layer = value;
}

static const char *Object_GetTag(UUID entityID) {
  const Entity &entity = GetEntity(entityID);

  if (!entity.HasComponent<TagComponent>())
    return "ANON";

  return entity.GetComponent<TagComponent>().Tag.c_str();
}

static void Object_SetTag(UUID entityID, const char *value) {
  const Entity &entity = GetEntity(entityID);

  if (!entity.HasComponent<TagComponent>())
    return;

  entity.GetComponent<TagComponent>().Tag = value;
}

#pragma endregion

#pragma region Camera

static void Camera_GetSize(UUID entityID, float *outSize) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    *outSize = 0.0f; // Perspective has no size
    break;
  case SceneCamera::ProjectionType::Orthographic:
    *outSize = cc.Camera.GetOrthographicSize();
    break;
  }
}

static void Camera_SetSize(UUID entityID, float *size) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  cc.Camera.SetOrthographicSize(*size);
}

static void Camera_GetFOV(UUID entityID, float *outFOV) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    *outFOV = cc.Camera.GetPerspectiveVerticalFov();
    break;
  case SceneCamera::ProjectionType::Orthographic:
    *outFOV = 0.0f; // Orthographic has no FOV
    break;
  }
}

static void Camera_SetFOV(UUID entityID, float *fov) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  cc.Camera.SetPerspectiveVerticalFov(*fov);
}

static void Camera_GetNear(UUID entityID, float *outNear) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    *outNear = cc.Camera.GetPerspectiveNearClip();
    break;
  case SceneCamera::ProjectionType::Orthographic:
    *outNear = cc.Camera.GetOrthographicNearClip();
    break;
  }
}

static void Camera_SetNear(UUID entityID, float *nearClip) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    cc.Camera.SetPerspectiveNearClip(*nearClip);
    break;
  case SceneCamera::ProjectionType::Orthographic:
    cc.Camera.SetOrthographicNearClip(*nearClip);
    break;
  }
}

static void Camera_GetFar(UUID entityID, float *outFar) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    *outFar = cc.Camera.GetPerspectiveFarClip();
    break;
  case SceneCamera::ProjectionType::Orthographic:
    *outFar = cc.Camera.GetOrthographicFarClip();
    break;
  }
}

static void Camera_SetFar(UUID entityID, float *farClip) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  switch (cc.Camera.GetProjectionType()) {
  case SceneCamera::ProjectionType::Perspectice:
    cc.Camera.SetPerspectiveFarClip(*farClip);
    break;
  case SceneCamera::ProjectionType::Orthographic:
    cc.Camera.SetOrthographicFarClip(*farClip);
    break;
  }
}

static void Camera_GetAspect(UUID entityID, float *outAspect) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  *outAspect = cc.Camera.GetAspectRatio();
}

static void Camera_SetAspect(UUID entityID, float *aspect) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  cc.Camera.SetAspectRatio(*aspect);
}

static void Camera_SetOrthographic(UUID entityID) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
}

static void Camera_SetPerspective(UUID entityID) {
  const Entity &entity = GetEntity(entityID);
  auto &cc = entity.GetComponent<CameraComponent>();
  cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspectice);
}

#pragma endregion
#pragma region Scripts

#pragma endregion

#pragma region Rigidbody2D

static void Rigidbody2D_ApplyForce(UUID entityID, glm::vec2 *impulse,
                                   glm::vec2 *point, bool wake) {
  const Entity &entity = GetEntity(entityID);
  Physics2D::Get().ApplyForce(entity, *impulse, *point, wake);
}
static void Rigidbody2D_ApplyForceToCenter(UUID entityID, glm::vec2 *impulse,
                                           bool wake) {
  const Entity &entity = GetEntity(entityID);
  Physics2D::Get().ApplyForceToCenter(entity, *impulse, wake);
}
static void Rigidbody2D_ApplyLinearImpulse(UUID entityID, glm::vec2 *impulse,
                                           glm::vec2 *point, bool wake) {
  const Entity &entity = GetEntity(entityID);
  Physics2D::Get().ApplyLinearImpulse(entity, *impulse, *point, wake);
}
static void Rigidbody2D_ApplyLinearImpulseToCenter(UUID entityID,
                                                   glm::vec2 *impulse,
                                                   bool wake) {
  const Entity &entity = GetEntity(entityID);
  Physics2D::Get().ApplyLinearImpulseToCenter(entity, *impulse, wake);
}

#pragma endregion

#pragma region BoxCollider2D

static void BoxCollider2D_GetOffset(UUID entityID, glm::vec2 *outOffset) {
  const Entity &entity = GetEntity(entityID);
  *outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
}

static void BoxCollider2D_SetOffset(UUID entityID, glm::vec2 *offset) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Offset = *offset;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    b2PolygonShape *shape = (b2PolygonShape *)fixture->GetShape();
    shape->SetAsBox(collider.Size.x * 0.5f, collider.Size.y * 0.5f,
                    b2Vec2(offset->x, offset->y), collider.Rotation);
  }
}

static void BoxCollider2D_GetSize(UUID entityID, glm::vec2 *outSize) {
  const Entity &entity = GetEntity(entityID);
  *outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
}

static void BoxCollider2D_SetSize(UUID entityID, glm::vec2 *size) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Size = *size;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    b2PolygonShape *shape = (b2PolygonShape *)fixture->GetShape();
    shape->SetAsBox(size->x * 0.5f, size->y * 0.5f,
                    b2Vec2(collider.Offset.x, collider.Offset.y),
                    collider.Rotation);

    // Recalculate body mass since size/area changed
    b2Body *body = fixture->GetBody();
    body->ResetMassData();
  }
}

static void BoxCollider2D_GetRotation(UUID entityID, float *outRotation) {
  const Entity &entity = GetEntity(entityID);
  *outRotation = entity.GetComponent<BoxCollider2DComponent>().Rotation;
}

static void BoxCollider2D_SetRotation(UUID entityID, float *rotation) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Rotation = *rotation;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    b2PolygonShape *shape = (b2PolygonShape *)fixture->GetShape();
    shape->SetAsBox(collider.Size.x * 0.5f, collider.Size.y * 0.5f,
                    b2Vec2(collider.Offset.x, collider.Offset.y), *rotation);
  }
}

static void BoxCollider2D_GetDensity(UUID entityID, float *outDensity) {
  const Entity &entity = GetEntity(entityID);
  *outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
}

static void BoxCollider2D_SetDensity(UUID entityID, float *density) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Density = *density;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetDensity(*density);

    // Density affects total body mass, reset it
    b2Body *body = fixture->GetBody();
    body->ResetMassData();
  }
}

static void BoxCollider2D_GetFriction(UUID entityID, float *outFriction) {
  const Entity &entity = GetEntity(entityID);
  *outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
}

static void BoxCollider2D_SetFriction(UUID entityID, float *friction) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Friction = *friction;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetFriction(*friction);
  }
}

// --- Restitution ---
static void BoxCollider2D_GetRestitution(UUID entityID, float *outRestitution) {
  const Entity &entity = GetEntity(entityID);
  *outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
}

static void BoxCollider2D_SetRestitution(UUID entityID, float *restitution) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.Restitution = *restitution;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetRestitution(*restitution);
  }
}

// --- RestitutionThreshold ---
static void BoxCollider2D_GetRestitutionThreshold(UUID entityID,
                                                  float *outThreshold) {
  const Entity &entity = GetEntity(entityID);
  *outThreshold =
      entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
}

static void BoxCollider2D_SetRestitutionThreshold(UUID entityID,
                                                  float *threshold) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<BoxCollider2DComponent>();
  collider.RestitutionThreshold = *threshold;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetRestitutionThreshold(*threshold);
  }
}

#pragma endregion

#pragma region CircleCollider2D

#pragma region CircleCollider2D

static void CircleCollider2D_GetOffset(UUID entityID, glm::vec2 *outOffset) {
  const Entity &entity = GetEntity(entityID);
  *outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
}

static void CircleCollider2D_SetOffset(UUID entityID, glm::vec2 *offset) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.Offset = *offset;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    b2CircleShape *shape = (b2CircleShape *)fixture->GetShape();
    shape->m_p.Set(offset->x, offset->y);
  }
}

static void CircleCollider2D_GetRadius(UUID entityID, float *outRadius) {
  const Entity &entity = GetEntity(entityID);
  *outRadius = entity.GetComponent<CircleCollider2DComponent>().Radius;
}

static void CircleCollider2D_SetRadius(UUID entityID, float *radius) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.Radius = *radius;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    b2CircleShape *shape = (b2CircleShape *)fixture->GetShape();
    shape->m_radius = *radius;

    // Recalculate body mass since radius/area changed
    b2Body *body = fixture->GetBody();
    body->ResetMassData();
  }
}

static void CircleCollider2D_GetDensity(UUID entityID, float *outDensity) {
  const Entity &entity = GetEntity(entityID);
  *outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
}

static void CircleCollider2D_SetDensity(UUID entityID, float *density) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.Density = *density;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetDensity(*density);

    // Density affects total body mass, reset it
    b2Body *body = fixture->GetBody();
    body->ResetMassData();
  }
}

static void CircleCollider2D_GetFriction(UUID entityID, float *outFriction) {
  const Entity &entity = GetEntity(entityID);
  *outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
}

static void CircleCollider2D_SetFriction(UUID entityID, float *friction) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.Friction = *friction;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetFriction(*friction);
  }
}

static void CircleCollider2D_GetRestitution(UUID entityID, float *outRestitution) {
  const Entity &entity = GetEntity(entityID);
  *outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
}

static void CircleCollider2D_SetRestitution(UUID entityID, float *restitution) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.Restitution = *restitution;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetRestitution(*restitution);
  }
}

static void CircleCollider2D_GetRestitutionThreshold(UUID entityID, float *outThreshold) {
  const Entity &entity = GetEntity(entityID);
  *outThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
}

static void CircleCollider2D_SetRestitutionThreshold(UUID entityID, float *threshold) {
  const Entity &entity = GetEntity(entityID);
  auto &collider = entity.GetComponent<CircleCollider2DComponent>();
  collider.RestitutionThreshold = *threshold;

  if (collider.RuntimeFixture) {
    b2Fixture *fixture = (b2Fixture *)collider.RuntimeFixture;
    fixture->SetRestitutionThreshold(*threshold);
  }
}

#pragma endregion

static void Transform_GetPosition(UUID entityID, glm::vec3 *outPosition) {
  const Entity &entity = GetEntity(entityID);
  *outPosition = entity.GetComponent<TransformComponent>().WorldPosition;
}

static void Transform_SetPosition(UUID entityID, glm::vec3 *position) {
  const Entity &entity = GetEntity(entityID);
  auto &transform = entity.GetComponent<TransformComponent>();

  // 1. Build a target world matrix using the new position,
  // while preserving the entity's current world rotation and scale.
  glm::mat4 targetWorldMatrix =
      glm::translate(glm::mat4(1.0), *position) *
      glm::toMat4(glm::quat(transform.WorldRotation)) *
      glm::scale(glm::mat4(1.0), transform.WorldScale);

  // 2. Get parent's world transform matrix (identity if no parent)
  glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
  if (entity.HasComponent<RelationshipComponent>()) {
    UUID parentID = entity.GetComponent<RelationshipComponent>().Parent;
    if (parentID != 0) {
      Entity parent = GetEntity(parentID);
      if (parent) {
        parentWorldMatrix = parent.GetWorldTransform();
      }
    }
  }

  // 3. Convert target world matrix to local space: Local = Inverse(ParentWorld)
  // * TargetWorld
  glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * targetWorldMatrix;

  // 4. Decompose the local matrix to update the local position component
  glm::vec3 localTranslation, localRotation, localScale;
  Math::DecomposeTransform(localMatrix, localTranslation, localRotation,
                           localScale);

  transform.LocalPosition = localTranslation;
}

static void Transform_GetRotation(UUID entityID, glm::vec3 *outRotation) {
  const Entity &entity = GetEntity(entityID);
  *outRotation = entity.GetComponent<TransformComponent>().WorldRotation;
}

static void Transform_SetRotation(UUID entityID, glm::vec3 *rotation) {
  const Entity &entity = GetEntity(entityID);
  auto &transform = entity.GetComponent<TransformComponent>();

  // 1. Build target world matrix with new rotation, preserving world position &
  // scale
  glm::mat4 targetWorldMatrix =
      glm::translate(glm::mat4(1.0f), transform.WorldPosition) *
      glm::toMat4(glm::quat(*rotation)) *
      glm::scale(glm::mat4(1.0f), transform.WorldScale);

  // 2. Get parent world matrix
  glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
  if (entity.HasComponent<RelationshipComponent>()) {
    UUID parentID = entity.GetComponent<RelationshipComponent>().Parent;
    if (parentID != 0) {
      Entity parent = GetEntity(parentID);
      if (parent) {
        parentWorldMatrix = parent.GetWorldTransform();
      }
    }
  }

  // 3. Transform to local space and decompose
  glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * targetWorldMatrix;

  glm::vec3 localTranslation, localRotation, localScale;
  Math::DecomposeTransform(localMatrix, localTranslation, localRotation,
                           localScale);

  transform.LocalRotation = glm::degrees(localRotation);
}

static void Transform_GetScale(UUID entityID, glm::vec3 *outScale) {
  const Entity &entity = GetEntity(entityID);
  *outScale = entity.GetComponent<TransformComponent>().WorldScale;
}

static void Transform_SetScale(UUID entityID, glm::vec3 *scale) {
  const Entity &entity = GetEntity(entityID);
  auto &transform = entity.GetComponent<TransformComponent>();

  // 1. Build target world matrix with new scale, preserving world position &
  // rotation
  glm::mat4 targetWorldMatrix =
      glm::translate(glm::mat4(1.0f), transform.WorldPosition) *
      glm::toMat4(glm::quat(transform.WorldRotation)) *
      glm::scale(glm::mat4(1.0f), *scale);

  // 2. Get parent world matrix
  glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
  if (entity.HasComponent<RelationshipComponent>()) {
    UUID parentID = entity.GetComponent<RelationshipComponent>().Parent;
    if (parentID != 0) {
      Entity parent = GetEntity(parentID);
      if (parent) {
        parentWorldMatrix = parent.GetWorldTransform();
      }
    }
  }

  // 3. Transform to local space and decompose
  glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * targetWorldMatrix;

  glm::vec3 localTranslation, localRotation, localScale;
  Math::DecomposeTransform(localMatrix, localTranslation, localRotation,
                           localScale);

  transform.LocalScale = localScale;
}

static void Transform_GetLocalPosition(UUID entityID, glm::vec3 *outPosition) {
  const Entity &entity = GetEntity(entityID);
  *outPosition = entity.GetComponent<TransformComponent>().LocalPosition;
}

static void Transform_SetLocalPosition(UUID entityID, glm::vec3 *position) {
  const Entity &entity = GetEntity(entityID);

  entity.GetComponent<TransformComponent>().LocalPosition = *position;
}

static void Transform_GetLocalRotation(UUID entityID, glm::vec3 *outRotation) {
  const Entity &entity = GetEntity(entityID);
  *outRotation = entity.GetComponent<TransformComponent>().LocalRotation;
}

static void Transform_SetLocalRotation(UUID entityID, glm::vec3 *rotation) {
  const Entity &entity = GetEntity(entityID);

  entity.GetComponent<TransformComponent>().LocalRotation = *rotation;
}

static void Transform_GetLocalScale(UUID entityID, glm::vec3 *outScale) {
  const Entity &entity = GetEntity(entityID);
  *outScale = entity.GetComponent<TransformComponent>().LocalScale;
}

static void Transform_SetLocalScale(UUID entityID, glm::vec3 *scale) {
  const Entity &entity = GetEntity(entityID);

  entity.GetComponent<TransformComponent>().LocalScale = *scale;
}

#pragma endregion

#pragma region Sprite

static void SpriteRenderer_GetColor(UUID entityID, glm::vec4 *outColor) {
  const Entity &entity = GetEntity(entityID);
  *outColor = entity.GetComponent<SpriteRendererComponent>().Color;
}

static void SpriteRenderer_SetColor(UUID entityID, glm::vec4 *color) {
  const Entity &entity = GetEntity(entityID);
  entity.GetComponent<SpriteRendererComponent>().Color = *color;
}

static void SpriteRenderer_GetTextureID(UUID entityID, uint64_t *outTextureID) {
  const Entity &entity = GetEntity(entityID);
  *outTextureID = entity.GetComponent<SpriteRendererComponent>().Texture.uuid();
}

static void SpriteRenderer_SetTextureID(UUID entityID, uint64_t *textureID) {
  const Entity &entity = GetEntity(entityID);
  entity.GetComponent<SpriteRendererComponent>().Texture = *textureID;
}

#pragma endregion

#pragma region CircleRenderer

static void CircleRenderer_GetColor(UUID entityID, glm::vec4 *outColor) {
  const Entity &entity = GetEntity(entityID);
  *outColor = entity.GetComponent<CircleRendererComponent>().Color;
}

static void CircleRenderer_SetColor(UUID entityID, glm::vec4 *color) {
  const Entity &entity = GetEntity(entityID);
  entity.GetComponent<CircleRendererComponent>().Color = *color;
}

static void CircleRenderer_GetThickness(UUID entityID, float *outThickness) {
  const Entity &entity = GetEntity(entityID);
  *outThickness = entity.GetComponent<CircleRendererComponent>().Thickness;
}

static void CircleRenderer_SetThickness(UUID entityID, float *thickness) {
  const Entity &entity = GetEntity(entityID);
  entity.GetComponent<CircleRendererComponent>().Thickness = *thickness;
}

static void CircleRenderer_GetFade(UUID entityID, float *outFade) {
  const Entity &entity = GetEntity(entityID);
  *outFade = entity.GetComponent<CircleRendererComponent>().Fade;
}

static void CircleRenderer_SetFade(UUID entityID, float *fade) {
  const Entity &entity = GetEntity(entityID);
  entity.GetComponent<CircleRendererComponent>().Fade = *fade;
}

#pragma endregion

#pragma region Editor Commands

static void Editor_ConsoleLog(int logLevel, const char *message) {
  auto console = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
  switch (logLevel) {
    case 1: // Trace (Green)
      console->WriteTrace(message);
      break;
    case 2: // Log (White)
      console->WriteLog(message);
      break;
    case 3: // Warning (Yellow)
      console->WriteWarning(message);
      break;
    case 4: // Error (Red)
      console->WriteError(message);
      break;
    case 5: // Critical (Dark Red text with Red background)
      console->WriteCritical(message);
      break;
    default:
      console->WriteLog(message);
  }
}


#pragma endregion

template <typename... Component> static void RegisterComponent() {

  (
      []() {
        const char *typeName = ComponentToString<Component>();

        std::string name(typeName);
        name = name.substr(0, name.length() - strlen("Component"));

        s_EntityHasComponentFuncs[name] = [](Entity entity) {
          return entity.HasComponent<Component>();
        };
      }(),
      ...);
}

template <typename... Component>
static void RegisterComponent(ComponentGroup<Component...>) {
  RegisterComponent<Component...>();
}

void ScriptGlue::RegisterComponents() {
  s_EntityHasComponentFuncs.clear();
  RegisterComponent(AllComponents{});
}

void ScriptGlue::RegisterFunctions() {
  CE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
  CE_ADD_INTERNAL_CALL(Input_IsKeyReleased);

#pragma region Object

  CE_ADD_INTERNAL_CALL(Object_HasComponent);
  CE_ADD_INTERNAL_CALL(Object_HasScript);
  CE_ADD_INTERNAL_CALL(Object_GetScript);
  CE_ADD_INTERNAL_CALL(Object_FindObjectByName);
  CE_ADD_INTERNAL_CALL(Object_GetLayer);
  CE_ADD_INTERNAL_CALL(Object_SetLayer);
  CE_ADD_INTERNAL_CALL(Object_GetTag);
  CE_ADD_INTERNAL_CALL(Object_SetTag);

#pragma endregion

#pragma region Camera
  CE_ADD_INTERNAL_CALL(Camera_GetSize);
  CE_ADD_INTERNAL_CALL(Camera_SetSize);
  CE_ADD_INTERNAL_CALL(Camera_GetFOV);
  CE_ADD_INTERNAL_CALL(Camera_SetFOV);
  CE_ADD_INTERNAL_CALL(Camera_GetNear);
  CE_ADD_INTERNAL_CALL(Camera_SetNear);
  CE_ADD_INTERNAL_CALL(Camera_GetAspect);
  CE_ADD_INTERNAL_CALL(Camera_SetAspect);
  CE_ADD_INTERNAL_CALL(Camera_GetFar);
  CE_ADD_INTERNAL_CALL(Camera_SetFar);
  CE_ADD_INTERNAL_CALL(Camera_SetOrthographic);
  CE_ADD_INTERNAL_CALL(Camera_SetPerspective);
#pragma endregion

#pragma region Scripts

#pragma endregion

#pragma region Rigidbody2D
  CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForce);
  CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyForceToCenter);
  CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulse);
  CE_ADD_INTERNAL_CALL(Rigidbody2D_ApplyLinearImpulseToCenter);
#pragma endregion

#pragma region BoxCollider2D
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetOffset);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetOffset);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetSize);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetSize);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetRotation);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetRotation);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetDensity);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetDensity);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetFriction);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetFriction);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetRestitution);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetRestitution);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_GetRestitutionThreshold);
  CE_ADD_INTERNAL_CALL(BoxCollider2D_SetRestitutionThreshold);
#pragma endregion

#pragma region CircleCollider2D
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetOffset);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetOffset);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetRadius);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetRadius);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetDensity);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetDensity);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetFriction);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetFriction);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetRestitution);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetRestitution);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_GetRestitutionThreshold);
  CE_ADD_INTERNAL_CALL(CircleCollider2D_SetRestitutionThreshold);
#pragma endregion

#pragma region Transform
  CE_ADD_INTERNAL_CALL(Transform_GetPosition);
  CE_ADD_INTERNAL_CALL(Transform_SetPosition);
  CE_ADD_INTERNAL_CALL(Transform_GetRotation);
  CE_ADD_INTERNAL_CALL(Transform_SetRotation);
  CE_ADD_INTERNAL_CALL(Transform_GetScale);
  CE_ADD_INTERNAL_CALL(Transform_SetScale);

  CE_ADD_INTERNAL_CALL(Transform_GetLocalPosition);
  CE_ADD_INTERNAL_CALL(Transform_SetLocalPosition);
  CE_ADD_INTERNAL_CALL(Transform_GetLocalRotation);
  CE_ADD_INTERNAL_CALL(Transform_SetLocalRotation);
  CE_ADD_INTERNAL_CALL(Transform_GetLocalScale);
  CE_ADD_INTERNAL_CALL(Transform_SetLocalScale);
#pragma endregion

#pragma region Sprite

  CE_ADD_INTERNAL_CALL(SpriteRenderer_GetColor);
  CE_ADD_INTERNAL_CALL(SpriteRenderer_SetColor);
  CE_ADD_INTERNAL_CALL(SpriteRenderer_GetTextureID);
  CE_ADD_INTERNAL_CALL(SpriteRenderer_SetTextureID);

#pragma endregion

#pragma region CircleRenderer
  CE_ADD_INTERNAL_CALL(CircleRenderer_GetColor);
  CE_ADD_INTERNAL_CALL(CircleRenderer_SetColor);
  CE_ADD_INTERNAL_CALL(CircleRenderer_GetThickness);
  CE_ADD_INTERNAL_CALL(CircleRenderer_SetThickness);
  CE_ADD_INTERNAL_CALL(CircleRenderer_GetFade);
  CE_ADD_INTERNAL_CALL(CircleRenderer_SetFade);
#pragma endregion

#pragma region Editor Commands
  CE_ADD_INTERNAL_CALL(Editor_ConsoleLog);
#pragma endregion

}
} // namespace CatEngine
