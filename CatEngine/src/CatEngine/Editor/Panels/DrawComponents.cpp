#include "cepch.h" // IWYU pragma: keep

#include "DrawComponents.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Core/Hash.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Scripting/ScriptEngine.h"

#include <imgui.h>

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
  if (!selection.HasComponent<TransformComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<TransformComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;
  
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(TransformComponent).hash_code()),
                                treeNodeFlags, "Transform");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##TransformSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("TransformSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("TransformSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = TransformComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();

    ImGuiDraw::Vec3("Position", component.LocalPosition, 0.1f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::degrees(component.LocalRotation);
    ImGuiDraw::Vec3("Rotation", rotation, 0.1f, 0.0f, 0.0f);
    component.LocalRotation = glm::radians(rotation);
    ImGuiDraw::Vec3("Scale", component.LocalScale, 0.1f, 0.0f, 0.0f);

    ImGui::TreePop();
  }
}

void DrawSpriteRendererComponent(Entity selection) {
  if (!selection.HasComponent<SpriteRendererComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<SpriteRendererComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(SpriteRendererComponent).hash_code()),
                                treeNodeFlags, "Sprite Renderer");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  // Right-aligned settings trigger on the component header
  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##SpriteRendererSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("SpriteRendererSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("SpriteRendererSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = SpriteRendererComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();

    // Resolve sprite asset label & optional texture for thumbnail preview
    std::string label = "None";
    Ref<Texture2D> previewTexture = nullptr;
    const AssetHandle handle = component.Texture;
    if (handle != 0 && AssetManager::IsAssetHandleValid(handle) &&
        AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
      const auto &metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
      label = metaData.AssetName.empty() ? metaData.FilePath.stem().string() : metaData.AssetName;
      previewTexture = AssetManager::GetAsset<Texture2D>(handle);
    }

    // Sprite object field row (labeled like Unity's "Sprite" property)
    ImGui::BeginGroup();
    ImGui::TextUnformatted("Sprite");
    ImGui::SameLine(110.0f); // Match your ImGuiDraw label column width

    const bool hasClearButton = (handle != 0);
    const float clearBtnWidth = hasClearButton ? (ImGui::GetFrameHeight() + 4.0f) : 0.0f;
    const float availFieldWidth = glm::max<float>(80.0f, ImGui::GetContentRegionAvail().x - clearBtnWidth);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.21f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.18f, 0.21f, 1.0f));

    if (ImGui::Button((label + "##SpriteSlotButton").c_str(), ImVec2(availFieldWidth, 0.0f))) {
      // Optional: ping/select asset in AssetBrowserPanel if desired
    }
    ImGui::PopStyleColor(3);

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
        const char *raw_str = static_cast<const char *>(payload->Data);
        size_t len = static_cast<size_t>(payload->DataSize);
        if (len > 0 && raw_str[len - 1] == '\0') {
          --len;
        }
        std::string_view sv(raw_str, len);
        uint64_t parsedHandle = 0;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), parsedHandle);

        // Support both raw AssetHandle binary payloads and string-encoded handles
        AssetHandle droppedHandle = 0;
        if (payload->DataSize == sizeof(AssetHandle)) {
          droppedHandle = *static_cast<const AssetHandle *>(payload->Data);
        } else if (ec == std::errc() && ptr == sv.data() + sv.size()) {
          droppedHandle = static_cast<AssetHandle>(parsedHandle);
        }

        if (droppedHandle != 0 && AssetManager::IsAssetHandleValid(droppedHandle)) {
          const AssetType type = AssetManager::GetAssetType(droppedHandle);
          if (type == AssetType::Texture2D) {
            component.Texture = droppedHandle;
          } else {
            CE_API_WARN("Cannot explicitly convert {0} to Texture2D", AssetTypeToString(type));
          }
        }
      }
      ImGui::EndDragDropTarget();
    }

    if (hasClearButton) {
      ImGui::SameLine(0.0f, 4.0f);
      if (ImGui::Button("x##ClearSprite", ImVec2(ImGui::GetFrameHeight(), 0.0f))) {
        component.Texture = 0;
      }
    }
    ImGui::EndGroup();

    ImGuiDraw::ColorEdit4("Color", component.Color);
    ImGuiDraw::Vec1("Tiling Factor", component.TilingFactor, 0.1f, 0.0f, 100.0f);

    if (previewTexture && previewTexture->GetWidth() > 0 && previewTexture->GetHeight() > 0) {
      ImGui::Spacing();
      const float thumbSize = 48.0f;
      const float aspect = static_cast<float>(previewTexture->GetWidth()) / static_cast<float>(previewTexture->GetHeight());
      ImVec2 drawSize = (aspect >= 1.0f)
        ? ImVec2(thumbSize, thumbSize / aspect)
        : ImVec2(thumbSize * aspect, thumbSize);

      ImGui::TextDisabled("Preview");
      ImGui::SameLine(110.0f);
      ImGui::Image((ImTextureID)((uint64_t)(previewTexture->GetRendererID())),
                   drawSize, ImVec2{0, 1}, ImVec2{1, 0});
    }

    ImGui::Spacing();
    ImGui::TreePop();
  }

  if (removeComponent) {
    selection.RemoveComponent<SpriteRendererComponent>();
  }
}

void DrawCircleRendererComponent(Entity selection) {
  if (!selection.HasComponent<CircleRendererComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<CircleRendererComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;
  
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(CircleRendererComponent).hash_code()),
                                treeNodeFlags, "Circle Renderer");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##CircleRendererSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("CircleRendererSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("CircleRendererSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = CircleRendererComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();
    ImGuiDraw::ColorEdit4("Color", component.Color);
    ImGuiDraw::Vec1("Thickness", component.Thickness, 0.025f, 0.f, 1.f);
    ImGuiDraw::Vec1("Fade", component.Fade, 0.00025f, 0.f, 1.f);
    ImGui::TreePop();
  }

  if (removeComponent)
    selection.RemoveComponent<CircleRendererComponent>();
}

void DrawCameraComponent(Entity selection) {
  if (!selection.HasComponent<CameraComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<CameraComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;
  
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(CircleRendererComponent).hash_code()),
                                treeNodeFlags, "Camera");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##CameraSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("CameraSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("CameraSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = CameraComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();
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
  if (!selection.HasComponent<Rigidbody2DComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<Rigidbody2DComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(CircleRendererComponent).hash_code()),
                                treeNodeFlags, "Rigidbody2D");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##Rigidbody2DSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("Rigidbody2DSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("Rigidbody2DSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = Rigidbody2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();    const char *bodyTypeString[] = {"Static", "Kinematic", "Dynamic"};
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
  if (!selection.HasComponent<BoxCollider2DComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<BoxCollider2DComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(BoxCollider2DComponent).hash_code()),
                                treeNodeFlags, "BoxCollider2D");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##BoxCollider2DSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("BoxCollider2DSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("BoxCollider2DSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = BoxCollider2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();

    ImGuiDraw::Vec2("Offset", component.Offset);
    ImGuiDraw::Vec2("Size", component.Size, 0.025f, 0.0001f, 100000.f);
    ImGuiDraw::Vec1("Rotation", component.Rotation, 0.025f, 0, 0);
    ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds);
    ImGuiDraw::CheckBox("IsTrigger", component.IsTrigger);
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
  if (!selection.HasComponent<CircleCollider2DComponent>()) {
    return;
  }

  auto &component = selection.GetComponent<CircleCollider2DComponent>();

  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  float lineHeight = ImGui::GetFrameHeight();
  bool open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeid(CircleCollider2DComponent).hash_code()),
                                treeNodeFlags, "CircleCollider2D");

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float buttonSize = lineHeight;
  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - buttonSize);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
  if (ImGui::Button(":##CircleCollider2DSettings", ImVec2(buttonSize, buttonSize))) {
    ImGui::OpenPopup("CircleCollider2DSettingsPopup");
  }
  ImGui::PopStyleColor(3);

  bool removeComponent = false;
  if (ImGui::BeginPopup("CircleCollider2DSettingsPopup")) {
    if (ImGui::MenuItem("Reset")) {
      component = CircleCollider2DComponent();
      ImGui::CloseCurrentPopup();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    ImGui::Spacing();
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
      label.append(" (" + field.TypeName + ")");
      ImGuiDraw::DataField("[" + std::to_string(index) + "]", label);
      break;
    }
    case DataType::Prefab: {
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
      label.append(" (" + field.TypeName + ")");
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
    default:
      break;
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
    labelName.append("(" + field.TypeName + ")");
    ImGuiDraw::DataField(field.Name, labelName);
    break;
    }
    case DataType::Prefab: {
      uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
      std::string labelName = Project::GetActive()
        ->GetEditorAssetManager()
        ->GetMetaData(data)
        .AssetName;
      labelName = labelName.empty() ? "(empty)" : labelName;
      labelName.append("(" + field.TypeName + ")");
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
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
          const char *raw_str = static_cast<const char *>(payload->Data);
          size_t len = static_cast<size_t>(payload->DataSize);
          if (len > 0 && raw_str[len - 1] == '\0') {
            --len;
          }

          std::string_view sv(raw_str, len);
          unsigned long value = 0;
          auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
          bool isAsset = (ec == std::errc() && ptr == sv.data() + sv.size());

          auto assetManager = Project::GetActive()->GetEditorAssetManager();
          if (!isAsset || !assetManager->IsAssetHandleValid(value)) {
            ImGui::EndDragDropTarget();
            break;
          }

          const Asset::MetaData &metaData = assetManager->GetMetaData(value);

          if (metaData.Type == AssetType::Texture2D) {
            *assetIDPtr = value;
          }
        }
        ImGui::EndDragDropTarget();
      }
      break;
    }
    case DataType::Prefab: {
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
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
          const char *raw_str = static_cast<const char *>(payload->Data);
          size_t len = static_cast<size_t>(payload->DataSize);
          if (len > 0 && raw_str[len - 1] == '\0') {
            --len;
          }

          std::string_view sv(raw_str, len);
          unsigned long value = 0;
          auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
          bool isAsset = (ec == std::errc() && ptr == sv.data() + sv.size());

          auto assetManager = Project::GetActive()->GetEditorAssetManager();
          if (!isAsset || !assetManager->IsAssetHandleValid(value)) {
            ImGui::EndDragDropTarget();
            break;
          }

          const Asset::MetaData &metaData = assetManager->GetMetaData(value);

          if (metaData.Type == AssetType::Prefab) {
            *assetIDPtr = value;
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
    default:
      break;
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
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
          const char *raw_str = static_cast<const char *>(payload->Data);
          size_t len = static_cast<size_t>(payload->DataSize);
          if (len > 0 && raw_str[len - 1] == '\0') {
            --len;
          }

          std::string_view sv(raw_str, len);
          unsigned long value = 0;
          auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
          bool isAsset = (ec == std::errc() && ptr == sv.data() + sv.size());

          auto assetManager = Project::GetActive()->GetEditorAssetManager();
          if (!isAsset || !assetManager->IsAssetHandleValid(value)) {
            ImGui::EndDragDropTarget();
            break;
          }

          const Asset::MetaData &metaData = assetManager->GetMetaData(value);

          if (metaData.Type == AssetType::Texture2D) {
            getOrCreateInstance().SetValue(value);
          }
        }
        ImGui::EndDragDropTarget();
      }
      break;
    }
    case DataType::Prefab: {
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
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
          const char *raw_str = static_cast<const char *>(payload->Data);
          size_t len = static_cast<size_t>(payload->DataSize);
          if (len > 0 && raw_str[len - 1] == '\0') {
            --len;
          }

          std::string_view sv(raw_str, len);
          unsigned long value = 0;
          auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
          bool isAsset = (ec == std::errc() && ptr == sv.data() + sv.size());

          auto assetManager = Project::GetActive()->GetEditorAssetManager();
          if (!isAsset || !assetManager->IsAssetHandleValid(value)) {
            ImGui::EndDragDropTarget();
            break;
          }

          const Asset::MetaData &metaData = assetManager->GetMetaData(value);

          if (metaData.Type == AssetType::Prefab) {
            getOrCreateInstance().SetValue(value);
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
  if (!selection.HasComponent<ScriptComponentContainer>()) {
    return;
  }

  auto &component = selection.GetComponent<ScriptComponentContainer>();

  const ImGuiTreeNodeFlags treeNodeFlags =
    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
    ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
    ImGuiTreeNodeFlags_FramePadding;

  auto &scriptEngine = ScriptEngine::GetMutable();
  auto context = scriptEngine.GetSceneContext();

  int idx = 0;

  std::vector<int> scriptsToRemove;

  for (auto &sc : component.Scripts) {
    int currentIdx = idx++;
    ImGui::PushID(currentIdx);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

    float lineHeight = ImGui::GetFrameHeight();
    bool open = ImGui::TreeNodeEx(&idx, treeNodeFlags, "Script");

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    const float buttonSize = lineHeight;
    const float rightEdge = ImGui::GetWindowContentRegionMax().x;
    ImGui::SameLine(rightEdge - buttonSize);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));
    if (ImGui::Button(":##ScriptSettings", ImVec2(buttonSize, buttonSize))) {
      ImGui::OpenPopup("ScriptSettingsPopup");
    }
    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopup("ScriptSettingsPopup")) {
      if (ImGui::MenuItem("Reset")) {
        sc = ScriptComponent();
        ImGui::CloseCurrentPopup();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Remove Component")) {
        scriptsToRemove.push_back(idx - 1);
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (open) {
      ImGui::Spacing();
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
    }

    ImGui::PopID();

  }

  for (auto it = scriptsToRemove.rbegin(); it != scriptsToRemove.rend(); ++it) {
    selection.RemoveScriptComponent(*it);
  }
}

} // namespace CatEngine
