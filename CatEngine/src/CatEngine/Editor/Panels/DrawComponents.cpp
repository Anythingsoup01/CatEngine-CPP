#include "DrawComponents.h"
#include "Capybara/Core/ReflectionTypes.h"
#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Core/Application.h"
#include "CatEngine/Core/Buffer.h"
#include "CatEngine/Core/Hash.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Components/3D/TransformComponent.h"
#include "CatEngine/Scene/Components/Scripting/ScriptComponent.h"
#include "CatEngine/Scene/Components/Scripting/ScriptComponentContainer.h"
#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scripting/ScriptEngine.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdint>

namespace CatEngine {

void DrawComponentsHelper(Entity selection) {
  DrawTransformComponent(selection);
  DrawSpriteRendererComponent(selection);
  DrawCircleRendererComponent(selection);
  DrawCameraComponent(selection);
  DrawRigidbody2DComponent(selection);
  DrawBoxCollider2DComponent(selection);
  DrawCircleCollider2DComponent(selection);
  DrawScriptComponent(selection);
}

void DrawTransformComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<TransformComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<TransformComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open = ImGui::TreeNodeEx((void *)typeid(TransformComponent).hash_code(),
                                treeNodeFlags, "Transform");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }
  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Reset Component")) {
      component = TransformComponent();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  if (open) {

    ImGuiDraw::Vec3("Position", component.LocalPosition, 0.1f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::degrees(component.LocalRotation);
    ImGuiDraw::Vec3("Rotation", rotation, 0.1f, 0.0f, 0.0f);
    component.LocalRotation = glm::radians(rotation);
    ImGuiDraw::Vec3("Scale", component.LocalScale, 0.1f, 0.0f, 0.0f);

    ImGui::TreePop();
  }
}

void DrawSpriteRendererComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<SpriteRendererComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<SpriteRendererComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open =
      ImGui::TreeNodeEx((void *)typeid(SpriteRendererComponent).hash_code(),
                        treeNodeFlags, "Sprite Renderer");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = SpriteRendererComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGuiDraw::ColorEdit4("Color", component.Color);

    std::string label = "None";
    AssetHandle handle = component.Texture;
    if (handle != 0) {
      if (AssetManager::IsAssetHandleValid(handle) &&
          AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
        const auto &metaData =
            Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
        std::string fileName = metaData.FilePath.filename().string();
        size_t extensionLen = metaData.FilePath.extension().string().length();
        fileName.erase(fileName.length() - extensionLen);
        label = fileName;
      }
    }

    ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
    buttonLabelSize.x = glm::max<float>(100.0f, buttonLabelSize.x);
    ImGui::Button(label.c_str(), buttonLabelSize);

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM")) {
        AssetHandle handle = *(AssetHandle *)payload->Data;
        const AssetType type = AssetManager::GetAssetType(handle);

        if (type == AssetType::Texture2D) {
          component.Texture = handle;
        } else {
          CE_API_WARN("Cannot explicitly convert {0} to Texture2D",
                      AssetTypeToString(type));
        }
      }
      ImGui::EndDragDropTarget();
    }
    if (strncmp(label.c_str(), "None", 4) != 0) {
      ImGui::SameLine();
      if (ImGui::Button("X")) {
        component.Texture = 0;
      }
    }

    ImGuiDraw::Vec1("Tiling Factor", component.TilingFactor, 0.1f, 0.f, 100.f);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<SpriteRendererComponent>();
}

void DrawCircleRendererComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<CircleRendererComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<CircleRendererComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open =
      ImGui::TreeNodeEx((void *)typeid(CircleRendererComponent).hash_code(),
                        treeNodeFlags, "Circle Renderer");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = CircleRendererComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGuiDraw::ColorEdit4("Color", component.Color);
    ImGuiDraw::Vec1("Thickness", component.Thickness, 0.025f, 0.f, 1.f);
    ImGuiDraw::Vec1("Fade", component.Fade, 0.00025f, 0.f, 1.f);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<CircleRendererComponent>();
}

void DrawCameraComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<CameraComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<CameraComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open = ImGui::TreeNodeEx((void *)typeid(CameraComponent).hash_code(),
                                treeNodeFlags, "Camera");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = CameraComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    auto &camera = component.Camera;

    const char *projectionTypeString[] = {"Perspective", "Orthographic"};
    const char *currentProjectionTypeString =
        projectionTypeString[(int)camera.GetProjectionType()];
    if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
      for (int i = 0; i < 2; i++) {
        bool isSelected =
            currentProjectionTypeString == projectionTypeString[i];
        if (ImGui::Selectable(projectionTypeString[i], &isSelected)) {
          currentProjectionTypeString = projectionTypeString[i];
          camera.SetProjectionType((SceneCamera::ProjectionType)i);
        }

        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }
    auto &primary = component.Primary;
    ImGuiDraw::CheckBox("Primary Camera", primary);

    if (camera.GetProjectionType() ==
        SceneCamera::ProjectionType::Perspectice) {
      auto vertFov = glm::degrees(camera.GetPerspectiveVerticalFov());
      if (ImGuiDraw::Vec1("Vertical FOV", vertFov))
        camera.SetPerspectiveVerticalFov(glm::radians(vertFov));

      auto nearClip = camera.GetPerspectiveNearClip();
      if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f, 0.f))
        camera.SetPerspectiveNearClip(nearClip);

      auto farClip = camera.GetPerspectiveFarClip();
      if (ImGuiDraw::Vec1("Far Clip", farClip, .01f, 0.f))
        camera.SetPerspectiveFarClip(farClip);
    }

    if (camera.GetProjectionType() ==
        SceneCamera::ProjectionType::Orthographic) {
      auto size = camera.GetOrthographicSize();
      if (ImGuiDraw::Vec1("FOV", size))
        camera.SetOrthographicSize(size);

      auto nearClip = camera.GetOrthographicNearClip();
      if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f, 0.f))
        camera.SetOrthographicNearClip(nearClip);

      auto farClip = camera.GetOrthographicFarClip();
      if (ImGuiDraw::Vec1("Far Clip", farClip, .01f, 0.f))
        camera.SetOrthographicFarClip(farClip);
    }

    auto &fixed = component.FixedAspectRatio;
    ImGuiDraw::CheckBox("Fixed Aspect Ratio", fixed);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<CameraComponent>();
}

void DrawRigidbody2DComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<Rigidbody2DComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<Rigidbody2DComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open =
      ImGui::TreeNodeEx((void *)typeid(Rigidbody2DComponent).hash_code(),
                        treeNodeFlags, "Rigidbody 2D");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = Rigidbody2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    const char *bodyTypeString[] = {"Static", "Kinematic", "Dynamic"};
    const char *currentBodyTypeString = bodyTypeString[(int)component.Type];
    if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
      for (int i = 0; i < 3; i++) {
        bool isSelected = currentBodyTypeString == bodyTypeString[i];
        if (ImGui::Selectable(bodyTypeString[i], &isSelected)) {
          currentBodyTypeString = bodyTypeString[i];
          component.Type = (Rigidbody2DComponent::BodyType)i;
        }

        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    ImGuiDraw::CheckBox("Fixed Rotation", component.FixedRotation);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<Rigidbody2DComponent>();
}

void DrawBoxCollider2DComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<BoxCollider2DComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<BoxCollider2DComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open =
      ImGui::TreeNodeEx((void *)typeid(BoxCollider2DComponent).hash_code(),
                        treeNodeFlags, "BoxCollider 2D");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = BoxCollider2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    glm::vec2 &offset = component.Offset;
    glm::vec2 &size = component.Size;

    float &density = component.Density;
    float &friction = component.Friction;
    float &restitution = component.Restitution;
    float &restitutionThreshold = component.RestitutionThreshold;

    ImGuiDraw::Vec2("Offset", component.Offset);
    ImGuiDraw::Vec2("Size", component.Size, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Rotation", component.Rotation, 0.025f, 0, 0);
    ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds);
    ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f,
                    100000.f);
    ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold,
                    0.025f, 0.0001f, 100000.f);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<BoxCollider2DComponent>();
}

void DrawCircleCollider2DComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<CircleCollider2DComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<CircleCollider2DComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open =
      ImGui::TreeNodeEx((void *)typeid(CircleCollider2DComponent).hash_code(),
                        treeNodeFlags, "CircleCollider 2D");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = CircleCollider2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    glm::vec2 &offset = component.Offset;
    float &size = component.Radius;

    float &density = component.Density;
    float &friction = component.Friction;
    float &restitution = component.Restitution;
    float &restitutionThreshold = component.RestitutionThreshold;

    ImGuiDraw::Vec2("Offset", component.Offset);
    ImGuiDraw::Vec1("Radius", component.Radius, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds);
    ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f,
                    100000.f);
    ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold,
                    0.025f, 0.0001f, 100000.f);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<CircleCollider2DComponent>();
}

void DrawRuntimeScriptVector(const ScriptField &field, const int &index,
                             void *elementPtr, Ref<Scene> context) {
  // Wrap the entire element drawing inside ImGui::BeginDisabled to block
  // interaction
  ImGui::BeginDisabled(true);

  switch (field.ElementType) {
  case DataType::Short: {
    int16_t val = *reinterpret_cast<int16_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::UShort: {
    uint16_t val = *reinterpret_cast<uint16_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::Int: {
    int32_t val = *reinterpret_cast<int32_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::UInt: {
    uint32_t val = *reinterpret_cast<uint32_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::Long: {
    int64_t val = *reinterpret_cast<int64_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::ULong: {
    uint64_t val = *reinterpret_cast<uint64_t *>(elementPtr);
    int intVal = static_cast<int>(val);
    ImGuiDraw::Int1("[" + std::to_string(index) + "]", intVal);
    break;
  }
  case DataType::Float: {
    float val = *reinterpret_cast<float *>(elementPtr);
    ImGuiDraw::Vec1("[" + std::to_string(index) + "]", val);
    break;
  }
  case DataType::Double: {
    double val = *reinterpret_cast<double *>(elementPtr);
    float floatVal = static_cast<float>(val);
    ImGuiDraw::Vec1("[" + std::to_string(index) + "]", floatVal);
    break;
  }
  case DataType::Bool: {
    bool val = *reinterpret_cast<bool *>(elementPtr);
    ImGuiDraw::CheckBox("[" + std::to_string(index) + "]", val);
    break;
  }
  case DataType::Vector2: {
    glm::vec2 val = *reinterpret_cast<glm::vec2 *>(elementPtr);
    ImGuiDraw::Vec2("[" + std::to_string(index) + "]", val);
    break;
  }
  case DataType::Vector3: {
    glm::vec3 val = *reinterpret_cast<glm::vec3 *>(elementPtr);
    ImGuiDraw::Vec3("[" + std::to_string(index) + "]", val);
    break;
  }
  case DataType::Vector4: {
    glm::vec4 val = *reinterpret_cast<glm::vec4 *>(elementPtr);
    ImGuiDraw::Vec4("[" + std::to_string(index) + "]", val);
    break;
  }
  case DataType::Entity: {
    uint64_t entityID = *reinterpret_cast<uint64_t *>(elementPtr);
    std::string labelName = (entityID != 0 && context)
                                ? context->GetEntityByUUID(entityID).GetName()
                                : "";
    labelName = labelName.empty() ? "(empty)" : labelName;
    labelName.append(" [" + std::to_string(index) + "]");

    ImGuiDraw::DataField("[" + std::to_string(index) + "]", labelName);
    break;
  }
  case DataType::Texture2D: {
    uint64_t assetID = *reinterpret_cast<uint64_t *>(elementPtr);
    std::string label = "";
    if (assetID != 0 && Project::GetActive() &&
        Project::GetActive()->GetEditorAssetManager()) {
      label = Project::GetActive()
                  ->GetEditorAssetManager()
                  ->GetMetaData(assetID)
                  .AssetName;
    }
    label = label.empty() ? "(empty)" : label;
    label.append(" [" + std::to_string(index) + "]");

    ImGuiDraw::DataField("[" + std::to_string(index) + "]", label);
    break;
  }
  case DataType::ScriptComponent: {
    void *scriptMemory = *reinterpret_cast<void **>(elementPtr);

    uint64_t currentEntityID = 0;
    if (scriptMemory) {
      currentEntityID = *reinterpret_cast<uint64_t *>(scriptMemory);
    }

    std::string entityName = "";
    std::string label = "";

    if (currentEntityID != 0 && context &&
        context->GetEntityByUUID(currentEntityID)) {
      entityName = context->GetEntityByUUID(currentEntityID).GetName();
      label = "(" + entityName + ") ";
    }

    label = label.empty() ? "(empty) " : label;
    label.append(" (" + field.ElementTypeName + ") [" + std::to_string(index) +
                 "]");

    ImGuiDraw::DataField("[" + std::to_string(index) + "]", label);
    break;
  }
  }

  ImGui::EndDisabled();
}

void DrawRuntimeScriptComponents(const ScriptField &field,
                                 Ref<ScriptInstance> scriptInstance,
                                 Ref<Scene> context) {
  switch (field.Type) {
  case DataType::Short: {
    int16_t data = scriptInstance->GetFieldData<int16_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, (int &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::UShort: {
    uint16_t data = scriptInstance->GetFieldData<uint16_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, (int &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Int: {
    int32_t data = scriptInstance->GetFieldData<int32_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::UInt: {
    uint32_t data = scriptInstance->GetFieldData<uint32_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, (int &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Long: {
    int64_t data = scriptInstance->GetFieldData<int64_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, (int &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::ULong: {
    uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
    if (ImGuiDraw::Int1(field.Name, (int &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Float: {
    float data = scriptInstance->GetFieldData<float>(field.Name);
    if (ImGuiDraw::Vec1(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Double: {
    double data = scriptInstance->GetFieldData<double>(field.Name);
    if (ImGuiDraw::Vec1(field.Name, (float &)data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Bool: {
    bool data = scriptInstance->GetFieldData<bool>(field.Name);
    if (ImGuiDraw::CheckBox(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Vector2: {
    glm::vec2 data = scriptInstance->GetFieldData<glm::vec2>(field.Name);
    if (ImGuiDraw::Vec2(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Vector3: {
    glm::vec3 data = scriptInstance->GetFieldData<glm::vec3>(field.Name);
    if (ImGuiDraw::Vec3(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::Vector4: {
    glm::vec4 data = scriptInstance->GetFieldData<glm::vec4>(field.Name);
    if (ImGuiDraw::Vec4(field.Name, data))
      scriptInstance->SetFieldData(field.Name, data);
    break;
  }
  case DataType::DynamicVector: {
    // Retrieve vector data live from the running script instance buffer
    VectorBuffer data = scriptInstance->GetVectorFieldData(field.Name);
    uint64_t count = data.GetCount();

    if (count != 0) {
      if (ImGui::TreeNodeEx(field.Name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
        for (size_t i = 0; i < count; ++i) {
          void *elementPtr = data.GetElementPointer(i, field.ElementSize);
          if (elementPtr) {
            DrawRuntimeScriptVector(field, static_cast<int>(i), elementPtr, context);
          }
        }
        ImGui::TreePop();
      }
    } else {
      ImGuiDraw::StaticString(field.Name, "(empty)");
    }
    break;
  }  case DataType::Entity: {
    uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
    std::string labelName = context->GetEntityByUUID(data).GetName();
    labelName = labelName.empty() ? "(empty)" : labelName;
    labelName.append(" (Entity)");
    ImGuiDraw::DataField(field.Name, labelName);
    break;
  }
  case DataType::Texture2D: {
    uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
    std::string labelName = Project::GetActive()
                                ->GetEditorAssetManager()
                                ->GetMetaData(data)
                                .AssetName;
    labelName = labelName.empty() ? "(empty)" : labelName;
    labelName.append(" (Texture2D)");
    ImGuiDraw::DataField(field.Name, labelName);
    break;
  }
  case DataType::ScriptComponent:
    uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
    std::string label = "";
    std::string entityName = "";

    entityName = context->GetEntityByUUID(data).GetName();
    label = "(" + entityName + ") ";

    label = label.empty() ? "(empty) " : label + "(" + field.TypeName + ")";
    ImGuiDraw::DataField(field.Name, label);
    break;
  }
}

bool DrawEditTimeScriptVector(const ScriptField &field, const int &index,
                              void *elementPtr, Ref<Scene> context,
                              Entity selection) {
  bool interaction = false;
  switch (field.ElementType) {
  case DataType::Short: {
    int16_t *data = reinterpret_cast<int16_t *>(elementPtr);
    int16_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::UShort: {
    uint16_t *data = reinterpret_cast<uint16_t *>(elementPtr);
    uint16_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Int: {
    int32_t *data = reinterpret_cast<int32_t *>(elementPtr);
    int32_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::UInt: {
    uint32_t *data = reinterpret_cast<uint32_t *>(elementPtr);
    uint32_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Long: {
    int64_t *data = reinterpret_cast<int64_t *>(elementPtr);
    int64_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::ULong: {
    uint64_t *data = reinterpret_cast<uint64_t *>(elementPtr);
    uint64_t val = *data;
    if (ImGuiDraw::Int1("[" + std::to_string(index) + "]", (int &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Float: {
    float *data = reinterpret_cast<float *>(elementPtr);
    float val = *data;
    if (ImGuiDraw::Vec1("[" + std::to_string(index) + "]", val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Double: {
    double *data = reinterpret_cast<double *>(elementPtr);
    double val = *data;
    if (ImGuiDraw::Vec1("[" + std::to_string(index) + "]", (float &)val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Bool: {
    bool *data = reinterpret_cast<bool *>(elementPtr);
    bool val = *data;
    if (ImGuiDraw::CheckBox("[" + std::to_string(index) + "]", val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Vector2: {
    glm::vec2 *data = reinterpret_cast<glm::vec2 *>(elementPtr);
    glm::vec2 val = *data;
    if (ImGuiDraw::Vec2("[" + std::to_string(index) + "]", val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Vector3: {
    glm::vec3 *data = reinterpret_cast<glm::vec3 *>(elementPtr);
    glm::vec3 val = *data;
    if (ImGuiDraw::Vec3("[" + std::to_string(index) + "]", val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Vector4: {
    glm::vec4 *data = reinterpret_cast<glm::vec4 *>(elementPtr);
    glm::vec4 val = *data;
    if (ImGuiDraw::Vec4("[" + std::to_string(index) + "]", val)) {
      *data = val;
      interaction = true;
    }
    break;
  }
  case DataType::Entity: {
    uint64_t *entityIDPtr = reinterpret_cast<uint64_t *>(elementPtr);
    uint64_t entityID = *entityIDPtr;

    std::string labelName = (entityID != 0 && context)
                                ? context->GetEntityByUUID(entityID).GetName()
                                : "";
    labelName = labelName.empty() ? "(empty)" : labelName;

    ImGuiDraw::DataField("[" + std::to_string(index) + "]", labelName);
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
        UUID droppedEntityID = *(UUID *)payload->Data;
        *entityIDPtr = droppedEntityID.uuid();
        interaction = true;
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  case DataType::Texture2D: {
    uint64_t *assetIDPtr = reinterpret_cast<uint64_t *>(elementPtr);
    uint64_t assetID = *assetIDPtr;

    std::string label = "";
    if (assetID != 0 && Project::GetActive() &&
        Project::GetActive()->GetEditorAssetManager()) {
      label = Project::GetActive()
                  ->GetEditorAssetManager()
                  ->GetMetaData(assetID)
                  .AssetName;
    }
    label = label.empty() ? "(empty)" : label;
    label.append(" [" + std::to_string(index) + "]");

    ImGuiDraw::DataField("[" + std::to_string(index) + "]", label);
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM")) {
        UUID droppedAssetID = *(UUID *)payload->Data;
        const auto &asset =
            Project::GetActive()->GetEditorAssetManager()->GetAsset(
                droppedAssetID);
        if (asset && asset->GetType() == AssetType::Texture2D) {
          *assetIDPtr = droppedAssetID.uuid();
          interaction = true;
        }
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  case DataType::ScriptComponent: {
    uint64_t *currentEntityID = reinterpret_cast<uint64_t *>(elementPtr);

    std::string entityName = "";
    std::string label = "";

    if (*currentEntityID != 0 && context &&
        context->GetEntityByUUID(*currentEntityID)) {
      entityName = context->GetEntityByUUID(*currentEntityID).GetName();
      label = "(" + entityName + ") ";
    }

    label = label.empty() ? "(empty)" : label;
    label.append(" (" + field.ElementTypeName + ")");
    // 1. Peek at the active drag payload to see if it matches this script type
    bool isValidDragTarget = false;
    if (const ImGuiPayload *activePayload = ImGui::GetDragDropPayload()) {
      if (activePayload->IsDataType("SCENE_HIERARCHY_ITEM") &&
          activePayload->Data) {
        UUID draggedID = *(UUID *)activePayload->Data;
        Entity draggedEntity = context->GetEntityByUUID(draggedID);

        // Note: Ensure 'selection' is passed into DrawEditTimeScriptVector if
        // you need it, or omit selection check if not required here.
        if (draggedEntity &&
            draggedEntity.HasComponent<ScriptComponentContainer>()) {
          auto &scc = draggedEntity.GetComponent<ScriptComponentContainer>();
          for (auto &script : scc.Scripts) {
            if (script.ClassName == field.ElementTypeName) {
              isValidDragTarget = true;
              break;
            }
          }
        }
      }
    }

    // 2. Apply visual highlight if a valid entity is being dragged over it
    if (isValidDragTarget) {
      ImGui::PushStyleColor(ImGuiCol_FrameBg,
                            ImVec4(0.15f, 0.55f, 0.25f, 0.8f));
    }

    // Draw ONCE using the index label
    ImGuiDraw::DataField("[" + std::to_string(index) + "]", label);

    if (isValidDragTarget) {
      ImGui::PopStyleColor();
    }

    // 3. Handle the drop acceptance with type validation
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
        UUID droppedEntityID = *(UUID *)payload->Data;
        Entity droppedEntity = context->GetEntityByUUID(droppedEntityID);

        if (droppedEntity) {
          bool matchesScript = false;
          if (droppedEntity.HasComponent<ScriptComponentContainer>()) {
            auto &scc = droppedEntity.GetComponent<ScriptComponentContainer>();
            for (auto &script : scc.Scripts) {
              if (script.ClassName == field.ElementTypeName) {
                matchesScript = true;
                break;
              }
            }
          }
          if (matchesScript) {
            *currentEntityID = droppedEntityID.uuid();
            interaction = true;
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  }
  return interaction;
}

void DrawEditTimeScriptComponents(const UUID &fieldID, const ScriptField &field,
                                  Ref<Scene> context, Entity selection,
                                  FieldMap &fieldMap) {
  bool hasInstance = (fieldMap.find(fieldID) != fieldMap.end());

  auto getOrCreateInstance = [&]() -> ScriptFieldInstance & {
    if (!hasInstance) {
      ScriptFieldInstance &sfi = fieldMap[fieldID];
      sfi.Field = field;
      if (field.IsVector) {
        sfi.VectorData = VectorBuffer(field.DefaultValue);
      } else {
        sfi.TrivialData.Allocate(field.Size);
        if (!field.DefaultValue.empty()) {
          sfi.SetValue(field.DefaultValue);
        }
      }
      hasInstance = true;
    }
    return fieldMap[fieldID];
  };

  switch (field.Type) {
  case DataType::Short: {
    int16_t data = hasInstance ? fieldMap[fieldID].GetValue<int16_t>()
                               : (!field.DefaultValue.empty()
                                      ? *(int16_t *)field.DefaultValue.data()
                                      : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::UShort: {
    uint16_t data = hasInstance ? fieldMap[fieldID].GetValue<uint16_t>()
                                : (!field.DefaultValue.empty()
                                       ? *(uint16_t *)field.DefaultValue.data()
                                       : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Int: {
    int32_t data = hasInstance ? fieldMap[fieldID].GetValue<int32_t>()
                               : (!field.DefaultValue.empty()
                                      ? *(int32_t *)field.DefaultValue.data()
                                      : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::UInt: {
    uint32_t data = hasInstance ? fieldMap[fieldID].GetValue<uint32_t>()
                                : (!field.DefaultValue.empty()
                                       ? *(uint32_t *)field.DefaultValue.data()
                                       : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Long: {
    int64_t data = hasInstance ? fieldMap[fieldID].GetValue<int64_t>()
                               : (!field.DefaultValue.empty()
                                      ? *(int64_t *)field.DefaultValue.data()
                                      : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::ULong: {
    uint64_t data = hasInstance ? fieldMap[fieldID].GetValue<uint64_t>()
                                : (!field.DefaultValue.empty()
                                       ? *(uint64_t *)field.DefaultValue.data()
                                       : 0);
    if (ImGuiDraw::Int1(field.Name, (int &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Float: {
    float data = hasInstance ? fieldMap[fieldID].GetValue<float>()
                             : (!field.DefaultValue.empty()
                                    ? *(float *)field.DefaultValue.data()
                                    : 0.0f);
    if (ImGuiDraw::Vec1(field.Name, data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Double: {
    double data = hasInstance ? fieldMap[fieldID].GetValue<double>()
                              : (!field.DefaultValue.empty()
                                     ? *(double *)field.DefaultValue.data()
                                     : 0.0);
    if (ImGuiDraw::Vec1(field.Name, (float &)data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Bool: {
    bool data = hasInstance ? fieldMap[fieldID].GetValue<bool>()
                            : (!field.DefaultValue.empty()
                                   ? *(bool *)field.DefaultValue.data()
                                   : false);
    if (ImGuiDraw::CheckBox(field.Name, data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Vector2: {
    glm::vec2 data = hasInstance
                         ? fieldMap[fieldID].GetValue<glm::vec2>()
                         : (!field.DefaultValue.empty()
                                ? *(glm::vec2 *)field.DefaultValue.data()
                                : glm::vec2(0.0f));
    if (ImGuiDraw::Vec2(field.Name, data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Vector3: {
    glm::vec3 data = hasInstance
                         ? fieldMap[fieldID].GetValue<glm::vec3>()
                         : (!field.DefaultValue.empty()
                                ? *(glm::vec3 *)field.DefaultValue.data()
                                : glm::vec3(0.0f));
    if (ImGuiDraw::Vec3(field.Name, data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::Vector4: {
    glm::vec4 data = hasInstance
                         ? fieldMap[fieldID].GetValue<glm::vec4>()
                         : (!field.DefaultValue.empty()
                                ? *(glm::vec4 *)field.DefaultValue.data()
                                : glm::vec4(0.0f));
    if (ImGuiDraw::Vec4(field.Name, data)) {
      getOrCreateInstance().SetValue(data);
    }
    break;
  }
  case DataType::DynamicVector: {
    VectorBuffer data = hasInstance ? fieldMap[fieldID].VectorData
                                    : (!field.DefaultValue.empty()
                                           ? VectorBuffer(field.DefaultValue)
                                           : VectorBuffer());

    uint64_t count = data.GetCount();
    if (count != 0) {
      std::string label = field.Name;
      if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
        for (size_t i = 0; i < count; ++i) {
          void *elementPtr = data.GetElementPointer(i, field.ElementSize);
          if (DrawEditTimeScriptVector(field, i, elementPtr, context,
                                       selection)) {
            getOrCreateInstance().SetVectorValue(data.Data);
          }
        }

        ImGui::TreePop();
      }
    } else {
      ImGuiDraw::StaticString(field.Name, "(empty)");
    }

    // --- Right-aligned Buttons ---
    // Calculate width for two small buttons (~20px each + spacing)
    float buttonWidth = 20.0f;
    float spacing = 4.0f;
    float totalWidth = (buttonWidth * 2.0f) + spacing;

    // Push the cursor over to the right side of the window/column
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x - totalWidth);

    // Render '+' button
    if (ImGui::Button("+", ImVec2(buttonWidth, 0))) {
      data.PushBackDefault(field.ElementSize);
      getOrCreateInstance().SetVectorValue(data.Data);
    }

    ImGui::SameLine(0.0f, spacing);
    // Disable '-' button if count is 0
    ImGui::BeginDisabled(count == 0);
    if (ImGui::Button("-", ImVec2(buttonWidth, 0))) {
      data.PopBack(field.ElementSize);
      getOrCreateInstance().SetVectorValue(data.Data);
    }
    ImGui::EndDisabled();
    break;
  }
  case DataType::Entity: {
    uint64_t entityID = hasInstance
                            ? fieldMap[fieldID].GetValue<uint64_t>()
                            : (!field.DefaultValue.empty()
                                   ? *(uint64_t *)field.DefaultValue.data()
                                   : 0);
    std::string labelName = (entityID != 0 && context)
                                ? context->GetEntityByUUID(entityID).GetName()
                                : "";
    labelName = labelName.empty() ? "(empty)" : labelName;
    labelName.append(" (" + field.TypeName + ")");

    ImGuiDraw::DataField(field.Name, labelName);
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
        UUID droppedEntityID = *(UUID *)payload->Data;
        getOrCreateInstance().SetValue(droppedEntityID.uuid());
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  case DataType::Texture2D: {
    uint64_t assetID = hasInstance
                           ? fieldMap[fieldID].GetValue<uint64_t>()
                           : (!field.DefaultValue.empty()
                                  ? *(uint64_t *)field.DefaultValue.data()
                                  : 0);
    std::string label = "";
    if (assetID != 0 && Project::GetActive() &&
        Project::GetActive()->GetEditorAssetManager()) {
      label = Project::GetActive()
                  ->GetEditorAssetManager()
                  ->GetMetaData(assetID)
                  .AssetName;
    }
    label = label.empty() ? "(empty)" : label;
    label.append(" (" + field.TypeName + ")");

    ImGuiDraw::DataField(field.Name, label);
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM")) {
        UUID droppedAssetID = *(UUID *)payload->Data;
        const auto &asset =
            Project::GetActive()->GetEditorAssetManager()->GetAsset(
                droppedAssetID);
        if (asset && asset->GetType() == AssetType::Texture2D) {
          getOrCreateInstance().SetValue(droppedAssetID.uuid());
        }
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  case DataType::ScriptComponent: {
    uint64_t currentEntityID =
        hasInstance ? fieldMap[fieldID].GetValue<uint64_t>()
                    : (!field.DefaultValue.empty()
                           ? *(uint64_t *)field.DefaultValue.data()
                           : 0);

    std::string label = "";
    std::string entityName = "";

    if (currentEntityID != 0 && context->GetEntityByUUID(currentEntityID)) {
      entityName = context->GetEntityByUUID(currentEntityID).GetName();
      label = "(" + entityName + ") ";
    }

    label = label.empty() ? "(empty) " : label;
    label.append("(" + field.TypeName + ")");

    // 1. Peek at the active drag payload to see if it matches this script
    // type
    bool isValidDragTarget = false;
    if (const ImGuiPayload *activePayload = ImGui::GetDragDropPayload()) {
      if (activePayload->IsDataType("SCENE_HIERARCHY_ITEM") &&
          activePayload->Data) {
        UUID draggedID = *(UUID *)activePayload->Data;
        Entity draggedEntity = context->GetEntityByUUID(draggedID);

        if (draggedEntity &&
            draggedEntity.HasComponent<ScriptComponentContainer>() &&
            draggedEntity.GetUUID() != selection.GetUUID()) {
          auto &scc = draggedEntity.GetComponent<ScriptComponentContainer>();
          for (auto &script : scc.Scripts) {
            // Check if the dragged entity has the exact script this field
            // expects
            if (script.ClassName == field.TypeName) {
              isValidDragTarget = true;
              break;
            }
          }
        }
      }
    }

    // 2. Apply visual highlight if a valid entity is currently being
    // dragged over it
    if (isValidDragTarget) {
      // Push a subtle green/accent tint to indicate a valid drop target
      ImGui::PushStyleColor(ImGuiCol_FrameBg,
                            ImVec4(0.15f, 0.55f, 0.25f, 0.8f));
    }

    ImGuiDraw::DataField(field.Name, label);

    if (isValidDragTarget) {
      ImGui::PopStyleColor();
    }

    // 3. Handle the drop acceptance with type validation
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
        UUID droppedEntityID = *(UUID *)payload->Data;
        Entity droppedEntity = context->GetEntityByUUID(droppedEntityID);

        if (droppedEntity) {
          // Double-check the script match on drop
          bool matchesScript = false;
          if (droppedEntity.HasComponent<ScriptComponentContainer>()) {
            auto &scc = droppedEntity.GetComponent<ScriptComponentContainer>();
            for (auto &script : scc.Scripts) {
              if (script.ClassName == field.TypeName) {
                matchesScript = true;
                break;
              }
            }
          }

          if (matchesScript) {
            getOrCreateInstance().SetValue(droppedEntityID);
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
    break;
  }
  }
}

void DrawScriptComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<ScriptComponentContainer>()) {
    return;
  }

  auto scriptEngine = ScriptEngine::GetMutable();
  auto context = scriptEngine.GetSceneContext();

  int idx = 0;
  auto &component = selection.GetComponent<ScriptComponentContainer>();

  std::vector<int> scriptsToRemove;

  for (auto &sc : component.Scripts) {
    int currentIdx = idx++;
    ImGui::PushID(currentIdx);
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
    float lineHeight =
        GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
    ImGui::Separator();
    bool open = ImGui::TreeNodeEx("Script", treeNodeFlags);
    ImGui::PopStyleVar();
    ImGui::SameLine(contentRegionAvailable.x - lineHeight);
    if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
      ImGui::OpenPopup("Component Settings");
    }

    bool removeComponent = false;
    if (ImGui::BeginPopup("Component Settings")) {
      if (ImGui::MenuItem("Remove Component")) {
        removeComponent = true;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::MenuItem("Reset Component")) {
        sc = ScriptComponent();
        auto &scriptFields = scriptEngine.GetScriptFieldMap(selection);
        if (scriptFields.find(sc.ScriptID) != scriptFields.end())
          scriptFields[sc.ScriptID.uuid()].clear();

        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (removeComponent) {
      scriptsToRemove.push_back(currentIdx);
      if (open)
        ImGui::TreePop();
      ImGui::PopID();
      continue;
    }

    if (!open) {
      ImGui::PopID();
      continue;
    }

    const auto &entityClasses = scriptEngine.GetScriptClasses();

    std::vector<std::string> classes = {"##EmptyScriptField"};
    for (auto &[_, scriptClass] : entityClasses) {
      std::string className = scriptClass->GetClassName();
      bool found = false;
      for (auto &loadedClass : component.LoadedScriptNames) {
        if (loadedClass == className)
          found = true;
      }

      if (!found)
        classes.push_back(scriptClass->GetClassName());
    }

    std::sort(classes.begin(), classes.end());

    std::string prevClassName = sc.ClassName;
    if (ImGuiDraw::Combo("Script", sc.ClassName, classes)) {

      if (prevClassName != sc.ClassName) {
        for (auto it = component.LoadedScriptNames.begin();
             it != component.LoadedScriptNames.end(); it++) {
          if (*it == prevClassName) {
            component.LoadedScriptNames.erase(it);
            component.LoadedScriptNames.push_back(sc.ClassName);
          }
        }
        sc.ScriptID =
            sc.ClassName.empty() ? 0 : Hash::GenerateFNVHash(sc.ClassName);
      }
    }

    sc.Loaded = true;

    bool scriptClassExists =
        scriptEngine.ScriptClassExists(sc.ClassName.c_str());
    bool sceneRunning = context->IsRunning();

    if (sceneRunning) {
      Ref<ScriptInstance> scriptInstance = scriptEngine.GetEntityScriptInstance(
          selection.GetUUID(), sc.ScriptID);

      if (!scriptInstance) {
        ImGui::TreePop();
        ImGui::PopID();
        continue;
      }

      const auto &fields = scriptInstance->GetScriptClass()->GetFields();

      for (const auto &[_, field] : fields) {
        DrawRuntimeScriptComponents(field, scriptInstance, context);
      }
      ImGui::TreePop();
      ImGui::PopID();
      continue;
    }

    if (!scriptClassExists) {
      ImGui::TreePop();
      ImGui::PopID();
      continue;
    }

    Ref<ScriptClass> scriptClass =
        scriptEngine.GetScriptClass(sc.ClassName.c_str());
    const auto &fields = scriptClass->GetFields();

    auto &scriptFields = scriptEngine.GetScriptFieldMap(selection);
    auto &fieldInstanceMap = scriptFields[sc.ScriptID];

    for (const auto &[fieldID, field] : fields) {
      DrawEditTimeScriptComponents(fieldID, field, context, selection,
                                   fieldInstanceMap);
    }
    ImGui::TreePop();
    ImGui::PopID();
  }

  for (auto it = scriptsToRemove.rbegin(); it != scriptsToRemove.rend(); ++it) {
    selection.RemoveScriptComponent(*it);
  }
}

} // namespace CatEngine
