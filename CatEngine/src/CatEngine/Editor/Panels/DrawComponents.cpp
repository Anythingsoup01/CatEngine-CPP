#include "DrawComponents.h"
#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Core/Application.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Components/3D/TransformComponent.h"
#include "CatEngine/Scene/Components/Scripting/ScriptComponent.h"
#include "CatEngine/Scripting/ScriptEngine.h"
#include "imgui.h"
#include "imgui_internal.h"

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
    ImGuiDraw::Vec3("Position", component.Position, 0.1f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::degrees(component.Rotation);
    ImGuiDraw::Vec3("Rotation", rotation, 0.1f, 0.0f, 0.0f);
    component.Rotation = glm::radians(rotation);
    ImGuiDraw::Vec3("Scale", component.Scale, 0.1f, 0.0f, 0.0f);
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

void DrawScriptComponent(Entity selection) {
  const ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  if (!selection.HasComponent<ScriptComponent>()) {
    return;
  }
  auto &component = selection.GetComponent<ScriptComponent>();
  ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{10, 4});
  float lineHeight =
      GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
  ImGui::Separator();
  bool open = ImGui::TreeNodeEx((void *)typeid(ScriptComponent).hash_code(),
                                treeNodeFlags, "Script");
  ImGui::PopStyleVar();
  ImGui::SameLine(contentRegionAvailable.x - lineHeight);
  if (ImGui::Button(":", ImVec2{lineHeight, lineHeight})) {
    ImGui::OpenPopup("Component Settings");
  }
  auto scriptEngine = ScriptEngine::GetMutable();
  auto context = scriptEngine.GetSceneContext();

  bool removeComponent = false;
  if (ImGui::BeginPopup("Component Settings")) {
    if (ImGui::MenuItem("Remove Component")) {
      removeComponent = true;
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Reset Component")) {
      component = ScriptComponent();
      auto &scriptFields = scriptEngine.GetScriptFieldMap(selection);
      scriptFields.clear();

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  if (open) {
    auto &sc = component;
    const auto &entityClasses = scriptEngine.GetScriptClasses();

    std::vector<const char *> classes = {""};
    for (auto &[_, scriptClass] : entityClasses) {
      classes.push_back(scriptClass->GetClassName());
    }

    if (ImGuiDraw::Combo("Class", sc.SelectedScript, classes)) {
      sc.ClassName = sc.SelectedScript != 0 ? classes.at(sc.SelectedScript) : "";
    }

    if (!sc.Loaded && !sc.ClassName.empty()) {
      int i = 0;
      for (auto &[_, scriptClass] : entityClasses) {
        i++;
        std::string scriptName = scriptClass->GetClassName();
        if (sc.ClassName == scriptName) {
          sc.SelectedScript = i;
        }
      }
    }

    sc.Loaded = true;

    bool scriptClassExists =
        scriptEngine.ScriptClassExists(sc.ClassName.c_str());
    bool sceneRunning = context->IsRunning();
    if (sceneRunning) {
      Ref<ScriptInstance> scriptInstance =
          scriptEngine.GetEntityScriptInstance(selection.GetUUID());
      if (scriptInstance) {
        const auto &fields = scriptInstance->GetScriptClass()->GetFields();

        for (const auto &[uuid, field] : fields) {
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
            glm::vec2 data =
                scriptInstance->GetFieldData<glm::vec2>(field.Name);
            if (ImGuiDraw::Vec2(field.Name, data))
              scriptInstance->SetFieldData(field.Name, data);
            break;
          }
          case DataType::Vector3: {
            glm::vec3 data =
                scriptInstance->GetFieldData<glm::vec3>(field.Name);
            if (ImGuiDraw::Vec3(field.Name, data))
              scriptInstance->SetFieldData(field.Name, data);
            break;
          }
          case DataType::Vector4: {
            glm::vec4 data =
                scriptInstance->GetFieldData<glm::vec4>(field.Name);
            if (ImGuiDraw::Vec4(field.Name, data))
              scriptInstance->SetFieldData(field.Name, data);
            break;
          }
          case DataType::Entity: {
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
          }
        }
      }
    } else // Scene isn't running
    {
      if (scriptClassExists) {
        Ref<ScriptClass> scriptClass =
            scriptEngine.GetScriptClass(component.ClassName.c_str());
        const auto &fields = scriptClass->GetFields();

        auto &scriptFields = scriptEngine.GetScriptFieldMap(selection);

        for (const auto &[uuid, field] : fields) {
          if (scriptFields.find(uuid) != scriptFields.end()) {
            // Display control to set it

            switch (field.Type) {
            case DataType::Short: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              int16_t data = scriptFieldInstance.GetValue<int16_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::UShort: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              uint16_t data = scriptFieldInstance.GetValue<uint16_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Int: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              int32_t data = scriptFieldInstance.GetValue<int32_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::UInt: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              uint32_t data = scriptFieldInstance.GetValue<uint32_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Long: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              int64_t data = scriptFieldInstance.GetValue<int64_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::ULong: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              uint64_t data = scriptFieldInstance.GetValue<uint64_t>();
              if (ImGuiDraw::Int1(field.Name, (int &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Float: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              float data = scriptFieldInstance.GetValue<float>();
              if (ImGuiDraw::Vec1(field.Name, data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Double: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              double data = scriptFieldInstance.GetValue<double>();
              if (ImGuiDraw::Vec1(field.Name, (float &)data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Bool: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              bool data = scriptFieldInstance.GetValue<bool>();
              if (ImGuiDraw::CheckBox(field.Name, data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Vector2: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              glm::vec2 data = scriptFieldInstance.GetValue<glm::vec2>();
              if (ImGuiDraw::Vec2(field.Name, data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Vector3: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              glm::vec3 data = scriptFieldInstance.GetValue<glm::vec3>();
              if (ImGuiDraw::Vec3(field.Name, data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Vector4: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              glm::vec4 data = scriptFieldInstance.GetValue<glm::vec4>();
              if (ImGuiDraw::Vec4(field.Name, data))
                scriptFieldInstance.SetValue(data);
              break;
            }

            case DataType::Entity: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              uint64_t data = scriptFieldInstance.GetValue<uint64_t>();

              std::string labelName = context->GetEntityByUUID(data).GetName();
              labelName.append(" (Entity)");
              ImGuiDraw::DataField(field.Name, labelName);

              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
                  UUID entityID = *(UUID *)payload->Data;
                  scriptFieldInstance.SetValue(entityID.uuid());
                }
                ImGui::EndDragDropTarget();
              }
              break;
            }

            case DataType::Texture2D: {
              ScriptFieldInstance &scriptFieldInstance = scriptFields.at(uuid);
              uint64_t data = scriptFieldInstance.GetValue<uint64_t>();

              std::string label = Project::GetActive()
                                      ->GetEditorAssetManager()
                                      ->GetMetaData(data)
                                      .AssetName;
              label.append(" (Texture2D)");
              ImGuiDraw::DataField(field.Name, label);

              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM")) {
                  UUID assetHandle = *(UUID *)payload->Data;
                  const auto &asset =
                      Project::GetActive()->GetEditorAssetManager()->GetAsset(
                          assetHandle);

                  if (asset->GetType() == AssetType::Texture2D)
                    scriptFieldInstance.SetValue(assetHandle);
                }
                ImGui::EndDragDropTarget();
              }
              break;
            }
            }
          } else // Script field instance isn't in the map
          {
            switch (field.Type) {
            case DataType::Short: {
              int16_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::UShort: {
              uint16_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Int: {
              int32_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::UInt: {
              uint32_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Long: {
              int64_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::ULong: {
              uint64_t data = 0;
              if (ImGuiDraw::Int1(field.Name, (int &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Float: {
              float data = 0.0f;
              if (ImGuiDraw::Vec1(field.Name, data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Double: {
              double data = 0.0;
              if (ImGuiDraw::Vec1(field.Name, (float &)data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Bool: {
              bool data = false;
              if (ImGuiDraw::CheckBox(field.Name, data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Vector2: {
              glm::vec2 data(0.0f);
              if (ImGuiDraw::Vec2(field.Name, data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Vector3: {
              glm::vec3 data(0.0f);
              if (ImGuiDraw::Vec3(field.Name, data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Vector4: {
              glm::vec4 data(0.0f);
              if (ImGuiDraw::Vec4(field.Name, data)) {
                ScriptFieldInstance &sfi = scriptFields[uuid];
                sfi.Field = field;
                sfi.SetValue(data);
              }
              break;
            }
            case DataType::Entity: {
              ImGuiDraw::DataField(field.Name, "Entity (empty)");
              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
                  UUID entityID = *(UUID *)payload->Data;
                  ScriptFieldInstance &sfi = scriptFields[uuid];
                  sfi.Field = field;
                  sfi.SetValue(entityID.uuid());
                }
                ImGui::EndDragDropTarget();
              }
              ImGui::PopItemWidth();
              break;
            }
            case DataType::Texture2D: {
              ImGuiDraw::DataField(field.Name, "Texture2D (empty)");
              if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload *payload =
                        ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM")) {
                  UUID assetID = *(UUID *)payload->Data;
                  const auto &asset =
                      Project::GetActive()->GetEditorAssetManager()->GetAsset(
                          assetID);
                  if (asset->GetType() == AssetType::Texture2D) {
                    ScriptFieldInstance &sfi = scriptFields[uuid];
                    sfi.Field = field;
                    sfi.SetValue(assetID.uuid());
                  }
                }
                ImGui::EndDragDropTarget();
              }
              break;
            }
            }
          }
        }
      }
    }
    ImGui::TreePop();
  }
  if (removeComponent)
    selection.RemoveComponent<ScriptComponent>();
}

} // namespace CatEngine
