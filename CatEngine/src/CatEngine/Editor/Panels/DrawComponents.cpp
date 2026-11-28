#include "cepch.h" // IWYU pragma: keep

#include "DrawComponents.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Core/Hash.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Scripting/ScriptEngine.h"

#include <imgui.h>

namespace CatEngine {

struct ComponentHeaderResult {
  bool Open = false;
  bool Reset = false;
  bool Remove = false;
};

// Same header style as the other drawers: framed tree node, dirty markers,
// and a ":" settings popup with Reset / Remove.
ComponentHeaderResult DrawComponentHeader(const char *name, size_t typeHash,
                                          bool prefabDirty, bool editDirty,
                                          bool isChild, bool removable = true) {
  const ImGuiTreeNodeFlags flags =
      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{8.0f, 5.0f});
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.19f, 0.20f, 0.23f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.24f, 0.28f, 1.0f));

  const float lineHeight = ImGui::GetFrameHeight();
  std::string label = name;
  if (prefabDirty && isChild)
    label += " P";
  if (editDirty)
    label += "*";

  ComponentHeaderResult result;
  result.Open = ImGui::TreeNodeEx(reinterpret_cast<void *>(typeHash), flags, "%s", label.c_str());

  ImGui::PopStyleColor(3);
  ImGui::PopStyleVar();

  const float rightEdge = ImGui::GetWindowContentRegionMax().x;
  ImGui::SameLine(rightEdge - lineHeight);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.32f, 0.36f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.36f, 0.38f, 0.42f, 0.9f));

  const std::string buttonID = ":##" + std::string(name) + "Settings";
  const std::string popupID = std::string(name) + "SettingsPopup";
  if (ImGui::Button(buttonID.c_str(), ImVec2(lineHeight, lineHeight)))
    ImGui::OpenPopup(popupID.c_str());
  ImGui::PopStyleColor(3);

  if (ImGui::BeginPopup(popupID.c_str())) {
    if (ImGui::MenuItem("Reset")) {
      result.Reset = true;
      ImGui::CloseCurrentPopup();
    }
    if (removable) {
      ImGui::Separator();
      if (ImGui::MenuItem("Remove Component")) {
        result.Remove = true;
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::EndPopup();
  }
  return result;
}

template <typename T>
ComponentHeaderResult BeginComponent(Entity selection, const char *name, T &component,
                                     bool &edited, bool removable = true) {
  const bool isChild = selection.GetComponent<RelationshipComponent>().Parent != 0;
  auto header = DrawComponentHeader(name, typeid(T).hash_code(), component.PrefabDirty,
                                    component.EditDirty, isChild, removable);
  if (header.Reset) {
    component = T();
    edited = true;
  }
  return header;
}

template <typename T>
void FinishComponent(Entity selection, T &component, bool edited,
                     const ComponentHeaderResult &header, const char *deletedName) {
  if (edited)
    MarkComponentEdited(selection, component);
  if (header.Remove)
    RemoveComponentTracked<T>(selection, deletedName);
}

template <typename T>
void MarkComponentEdited(Entity selection, T &component) {
  component.EditDirty = true;
  auto &rc = selection.GetComponent<RelationshipComponent>();
  if (rc.Parent != 0 && selection.GetRootParent().HasComponent<PrefabLinkComponent>())
    component.PrefabDirty = true;
}

template <typename T>
void RemoveComponentTracked(Entity selection, const char *deletedName) {
  selection.RemoveComponent<T>();
  selection.GetComponent<ObjectStateComponent>().DeletedComponents.push_back(deletedName);
}

template <typename E>
bool DrawEnumCombo(const char *label, E &value, const char *const *names, int count) {
  bool changed = false;
  if (ImGui::BeginCombo(label, names[(int)value])) {
    for (int i = 0; i < count; i++) {
      const bool selected = (int)value == i;
      if (ImGui::Selectable(names[i], selected)) {
        value = (E)i;
        changed = true;
      }
      if (selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}

// Texture slot with asset drag-and-drop and a clear button.
// Same behavior as the sprite renderer's slot.
bool DrawTextureSlot(const char *label, AssetHandle &texture) {
  bool changed = false;

  std::string name = "None";
  if (texture != 0 && AssetManager::IsAssetHandleValid(texture) &&
      AssetManager::GetAssetType(texture) == AssetType::Texture2D) {
    const auto &md = Project::GetActive()->GetEditorAssetManager()->GetMetaData(texture);
    name = md.AssetName.empty() ? md.FilePath.stem().string() : md.AssetName;
  }

  ImGui::PushID(label);
  ImGui::BeginGroup();
  ImGui::TextUnformatted(label);
  ImGui::SameLine(110.0f);

  const bool hasClear = texture != 0;
  const float clearW = hasClear ? (ImGui::GetFrameHeight() + 4.0f) : 0.0f;
  const float fieldW = glm::max<float>(80.0f, ImGui::GetContentRegionAvail().x - clearW);

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.21f, 0.25f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.18f, 0.21f, 1.0f));
  ImGui::Button((name + "##Slot").c_str(), ImVec2(fieldW, 0.0f));
  ImGui::PopStyleColor(3);

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
      const char *raw = static_cast<const char *>(payload->Data);
      size_t len = static_cast<size_t>(payload->DataSize);
      if (len > 0 && raw[len - 1] == '\0')
        --len;
      std::string_view sv(raw, len);
      uint64_t parsed = 0;
      auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), parsed);

      AssetHandle dropped = 0;
      if (payload->DataSize == sizeof(AssetHandle))
        dropped = *static_cast<const AssetHandle *>(payload->Data);
      else if (ec == std::errc() && ptr == sv.data() + sv.size())
        dropped = static_cast<AssetHandle>(parsed);

      if (dropped != 0 && AssetManager::IsAssetHandleValid(dropped)) {
        const AssetType type = AssetManager::GetAssetType(dropped);
        if (type == AssetType::Texture2D) {
          texture = dropped;
          changed = true;
        } else {
          CE_API_WARN("Cannot explicitly convert {0} to Texture2D", AssetTypeToString(type));
        }
      }
    }
    ImGui::EndDragDropTarget();
  }

  if (hasClear) {
    ImGui::SameLine(0.0f, 4.0f);
    if (ImGui::Button("x##Clear", ImVec2(ImGui::GetFrameHeight(), 0.0f))) {
      texture = 0;
      changed = true;
    }
  }
  ImGui::EndGroup();
  ImGui::PopID();
  return changed;
}


void DrawComponentsHelper(Entity selection) {
  if (selection.HasComponent<RectTransformComponent>())
    DrawRectTransformComponent(selection);
  else
    DrawTransformComponent(selection);

  DrawCanvasComponent(selection);
  DrawCanvasGroupComponent(selection);
  DrawPanelComponent(selection);
  DrawButtonComponent(selection);
  DrawSelectableComponent(selection);
  DrawEventListenerComponent(selection);
  DrawSpriteRendererComponent(selection);
  DrawCircleRendererComponent(selection);
  DrawCameraComponent(selection);
  DrawRigidbody2DComponent(selection);
  DrawBoxCollider2DComponent(selection);
  DrawCircleCollider2DComponent(selection);
  DrawScriptComponent(selection);
}

void DrawTransformComponent(Entity selection) {
  if (!selection.HasComponent<TransformComponent>())
    return;

  auto &component = selection.GetComponent<TransformComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Transform", component, edited, /*removable*/ false);

  if (header.Open) {
    ImGui::Spacing();

    if (ImGuiDraw::Vec3("Position", component.LocalPosition, 0.1f, 0.0f, 0.0f))
      edited = true;

    glm::vec3 rotation = glm::degrees(component.LocalRotation);
    if (ImGuiDraw::Vec3("Rotation", rotation, 0.1f, 0.0f, 0.0f)) {
      component.LocalRotation = glm::radians(rotation);
      edited = true;
    }

    if (ImGuiDraw::Vec3("Scale", component.LocalScale, 0.1f, 0.0f, 0.0f))
      edited = true;

    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Transform");
}

void DrawRectTransformComponent(Entity selection) {
  if (!selection.HasComponent<RectTransformComponent>())
    return;

  const bool inCanvas = selection.GetScene()->IsUnderCanvas(selection);
  auto &component = selection.GetComponent<RectTransformComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "RectTransform", component, edited, !inCanvas);

  if (header.Open) {
    ImGui::Spacing();

    struct AnchorOption { const char *Name; UIAnchorPreset Preset; };
    static const AnchorOption options[] = {
        {"Top Left", UIAnchorPreset::TopLeft},
        {"Top Center", UIAnchorPreset::TopCenter},
        {"Top Right", UIAnchorPreset::TopRight},
        {"Middle Left", UIAnchorPreset::MiddleLeft},
        {"Center", UIAnchorPreset::Center},
        {"Middle Right", UIAnchorPreset::MiddleRight},
        {"Bottom Left", UIAnchorPreset::BottomLeft},
        {"Bottom Center", UIAnchorPreset::BottomCenter},
        {"Bottom Right", UIAnchorPreset::BottomRight},
        {"Stretch All", UIAnchorPreset::StretchAll},
        {"Custom", UIAnchorPreset::Custom},
    };

    const char *current = "Custom";
    for (const auto &o : options)
      if (o.Preset == component.AnchorPreset)
        current = o.Name;

    if (ImGui::BeginCombo("Anchor Preset", current)) {
      for (const auto &o : options) {
        const bool selected = o.Preset == component.AnchorPreset;
        if (ImGui::Selectable(o.Name, selected) && o.Preset != UIAnchorPreset::Custom) {
          component.ApplyAnchorPreset(o.Preset);
          // A stretched element fills its parent: clear the offsets.
          if (o.Preset == UIAnchorPreset::StretchAll) {
            component.Size = {0.0f, 0.0f};
            component.Position = {0.0f, 0.0f};
          }
          edited = true;
        }
        if (selected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    bool anchorsEdited = false;
    if (ImGuiDraw::Vec2("Anchor Min", component.AnchorMin, 0.01f, 0.0f, 1.0f))
      anchorsEdited = true;
    if (ImGuiDraw::Vec2("Anchor Max", component.AnchorMax, 0.01f, 0.0f, 1.0f))
      anchorsEdited = true;
    if (anchorsEdited) {
      component.AnchorPreset = UIAnchorPreset::Custom;
      edited = true;
    }

    if (ImGuiDraw::Vec2("Pivot", component.Pivot, 0.01f, 0.0f, 1.0f))
      edited = true;

    const bool stretched = component.AnchorMin != component.AnchorMax;
    if (ImGuiDraw::Vec2("Position", component.Position, 0.5f, 0.0f, 0.0f))
      edited = true;
    if (ImGuiDraw::Vec2(stretched ? "Size Delta" : "Size", component.Size, 0.5f, 0.0f, 0.0f))
      edited = true;
    if (stretched)
      ImGui::TextDisabled("Size Delta is added to the stretched size (0 = fill).");

    if (ImGuiDraw::Vec1("Rotation", component.Rotation, 0.1f, 0.0f, 0.0f))
      edited = true;

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "RectTransform");
}

void DrawCanvasComponent(Entity selection) {
  if (!selection.HasComponent<CanvasComponent>())
    return;

  auto &component = selection.GetComponent<CanvasComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Canvas", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    static const char *renderModes[] = {"Screen Space - Overlay"};
    if (DrawEnumCombo("Render Mode", component.RenderMode, renderModes, 1))
      edited = true;

    if (ImGuiDraw::Int1("Sort Order", component.SortOrder))
      edited = true;

    ImGui::Separator();

    static const char *scaleModes[] = {"Constant Pixel Size", "Scale With Screen Size"};
    if (DrawEnumCombo("UI Scale Mode", component.ScaleMode, scaleModes, 2))
      edited = true;

    if (component.ScaleMode == CanvasScaleMode::ConstantPixelSize) {
      if (ImGuiDraw::Vec1("Scale Factor", component.ScaleFactor, 0.01f, 0.01f, 100.0f))
        edited = true;
    } else {
      if (ImGuiDraw::Vec2("Reference Resolution", component.ReferenceResolution, 1.0f, 1.0f, 16384.0f))
        edited = true;

      static const char *matchModes[] = {"Match Width Or Height", "Expand", "Shrink"};
      if (DrawEnumCombo("Screen Match Mode", component.ScreenMatchMode, matchModes, 3))
        edited = true;

      if (component.ScreenMatchMode == CanvasScreenMatchMode::MatchWidthOrHeight) {
        if (ImGui::SliderFloat("Match", &component.MatchWidthOrHeight, 0.0f, 1.0f, "%.2f"))
          edited = true;
        ImGui::TextDisabled("0 = Width      1 = Height");
      }
    }

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Canvas");
}

void DrawCanvasGroupComponent(Entity selection) {
  if (!selection.HasComponent<CanvasGroupComponent>())
    return;

  auto &component = selection.GetComponent<CanvasGroupComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "CanvasGroup", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (ImGui::SliderFloat("Alpha", &component.Alpha, 0.0f, 1.0f, "%.2f"))
      edited = true;
    if (ImGuiDraw::CheckBox("Interactable", component.Interactable))
      edited = true;
    if (ImGuiDraw::CheckBox("Blocks Raycasts", component.BlocksRaycasts))
      edited = true;
    if (ImGuiDraw::CheckBox("Ignore Parent Groups", component.IgnoreParentGroups))
      edited = true;

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Canvas Group");
}

void DrawPanelComponent(Entity selection) {
  if (!selection.HasComponent<PanelComponent>())
    return;

  auto &component = selection.GetComponent<PanelComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Panel", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (DrawTextureSlot("Image", component.Texture))
      edited = true;
    if (ImGuiDraw::ColorEdit4("Color", component.Color))
      edited = true;
    if (ImGuiDraw::CheckBox("Raycast Target", component.RaycastTarget))
      edited = true;

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Panel");
}

void DrawButtonComponent(Entity selection) {
  if (!selection.HasComponent<ButtonVisualComponent>())
    return;

  auto &component = selection.GetComponent<ButtonVisualComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Button", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (DrawTextureSlot("Image", component.Texture))
      edited = true;
    if (ImGuiDraw::ColorEdit4("Normal Color", component.NormalColor))
      edited = true;
    if (ImGuiDraw::CheckBox("Raycast Target", component.RaycastTarget))
      edited = true;

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Button");
}

void DrawSelectableComponent(Entity selection) {
  if (!selection.HasComponent<SelectableComponent>())
    return;

  auto &component = selection.GetComponent<SelectableComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Selectable", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (ImGuiDraw::CheckBox("Interactable", component.Interactable))
      edited = true;
    if (ImGuiDraw::CheckBox("Navigable", component.Navigable))
      edited = true;

    static const char *transitions[] = {"None", "Color Tint"};
    if (DrawEnumCombo("Transition", component.Transition, transitions, 2))
      edited = true;

    if (component.Transition == UITransition::ColorTint) {
      if (ImGui::TreeNodeEx("Colors", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        if (ImGuiDraw::ColorEdit4("Normal", component.NormalTint)) edited = true;
        if (ImGuiDraw::ColorEdit4("Highlighted", component.HighlightedTint)) edited = true;
        if (ImGuiDraw::ColorEdit4("Pressed", component.PressedTint)) edited = true;
        if (ImGuiDraw::ColorEdit4("Selected", component.SelectedTint)) edited = true;
        if (ImGuiDraw::ColorEdit4("Disabled", component.DisabledTint)) edited = true;
        ImGui::TreePop();
      }
      if (ImGuiDraw::Vec1("Fade Duration", component.FadeDuration, 0.01f, 0.0f, 10.0f))
        edited = true;
      if (ImGuiDraw::CheckBox("Tint Children", component.TintChildren))
        edited = true;
    }

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Selectable");
}

void DrawEventListenerComponent(Entity selection) {
  if (!selection.HasComponent<EventListenerComponent>())
    return;

  auto &component = selection.GetComponent<EventListenerComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "EventListener", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    struct MaskBit { const char *Name; uint32_t Bit; };
    static const MaskBit bits[] = {
        {"Hover", UIMask::Hover},
        {"Press", UIMask::Press},
        {"Click", UIMask::Click},
        {"Drag", UIMask::Drag},
        {"Scroll", UIMask::Scroll},
    };

    for (const auto &b : bits) {
      bool on = (component.Mask & b.Bit) != 0;
      if (ImGuiDraw::CheckBox(b.Name, on)) {
        component.Mask = on ? (component.Mask | b.Bit) : (component.Mask & ~b.Bit);
        edited = true;
      }
    }

    if (!selection.HasComponent<RectTransformComponent>())
      ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f),
                         "Needs a RectTransform to receive events.");

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Event Listener");
}

void DrawSpriteRendererComponent(Entity selection) {
  if (!selection.HasComponent<SpriteRendererComponent>())
    return;

  auto &component = selection.GetComponent<SpriteRendererComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "SpriteRenderer", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (DrawTextureSlot("Sprite", component.Texture))
      edited = true;
    if (ImGuiDraw::ColorEdit4("Color", component.Color))
      edited = true;
    if (ImGuiDraw::Vec1("Tiling Factor", component.TilingFactor, 0.1f, 0.0f, 100.0f))
      edited = true;

    const AssetHandle handle = component.Texture;
    if (handle != 0 && AssetManager::IsAssetHandleValid(handle) &&
        AssetManager::GetAssetType(handle) == AssetType::Texture2D) {
      Ref<Texture2D> preview = AssetManager::GetAsset<Texture2D>(handle);
      if (preview && preview->GetWidth() > 0 && preview->GetHeight() > 0) {
        ImGui::Spacing();
        const float thumb = 48.0f;
        const float aspect = (float)preview->GetWidth() / (float)preview->GetHeight();
        const ImVec2 size = aspect >= 1.0f ? ImVec2(thumb, thumb / aspect)
                                           : ImVec2(thumb * aspect, thumb);
        ImGui::TextDisabled("Preview");
        ImGui::SameLine(110.0f);
        ImGui::Image((ImTextureID)((uint64_t)preview->GetRendererID()), size,
                     ImVec2{0, 1}, ImVec2{1, 0});
      }
    }

    ImGui::Spacing();
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Sprite Renderer");
}

void DrawCircleRendererComponent(Entity selection) {
  if (!selection.HasComponent<CircleRendererComponent>())
    return;

  auto &component = selection.GetComponent<CircleRendererComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "CircleRenderer", component, edited);

  if (header.Open) {
    ImGui::Spacing();
    if (ImGuiDraw::ColorEdit4("Color", component.Color))
      edited = true;
    if (ImGuiDraw::Vec1("Thickness", component.Thickness, 0.025f, 0.f, 1.f))
      edited = true;
    if (ImGuiDraw::Vec1("Fade", component.Fade, 0.00025f, 0.f, 1.f))
      edited = true;
    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Circle Renderer");
}

void DrawCameraComponent(Entity selection) {
  if (!selection.HasComponent<CameraComponent>())
    return;

  auto &component = selection.GetComponent<CameraComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Camera", component, edited);

  if (header.Open) {
    ImGui::Spacing();
    auto &camera = component.Camera;

    static const char *projectionNames[] = {"Perspective", "Orthographic"};
    const int currentProjection = (int)camera.GetProjectionType();
    if (ImGui::BeginCombo("Projection", projectionNames[currentProjection])) {
      for (int i = 0; i < 2; i++) {
        const bool selected = currentProjection == i;
        if (ImGui::Selectable(projectionNames[i], selected)) {
          camera.SetProjectionType((SceneCamera::ProjectionType)i);
          edited = true;
        }
        if (selected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGuiDraw::CheckBox("Active", component.Active)) edited = true;
    if (ImGuiDraw::CheckBox("Primary Camera", component.Primary)) edited = true;

    const bool ortho = camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic;
    if (!ortho) {
      float vertFov = glm::degrees(camera.GetPerspectiveVerticalFov());
      if (ImGuiDraw::Vec1("Vertical FOV", vertFov)) { camera.SetPerspectiveVerticalFov(glm::radians(vertFov)); edited = true; }
      float nearClip = camera.GetPerspectiveNearClip();
      if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f, 0.f)) { camera.SetPerspectiveNearClip(nearClip); edited = true; }
      float farClip = camera.GetPerspectiveFarClip();
      if (ImGuiDraw::Vec1("Far Clip", farClip, .01f, 0.f)) { camera.SetPerspectiveFarClip(farClip); edited = true; }
    } else {
      static const char *sizeModes[] = {"Units", "Pixels Per Unit"};
      if (DrawEnumCombo("Size Mode", component.OrthoSizeMode, sizeModes, 2)) edited = true;

      if (component.OrthoSizeMode == CameraSizeMode::PixelsPerUnit) {
        if (ImGuiDraw::Vec1("Pixels Per Unit", component.PixelsPerUnit, 1.0f, 1.0f, 10000.0f)) edited = true;
        ImGui::TextDisabled("World size = output height / pixels per unit.");
      } else {
        float size = camera.GetOrthographicSize();
        if (ImGuiDraw::Vec1("Size", size)) { camera.SetOrthographicSize(size); edited = true; }
      }
      float nearClip = camera.GetOrthographicNearClip();
      if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f)) { camera.SetOrthographicNearClip(nearClip); edited = true; }
      float farClip = camera.GetOrthographicFarClip();
      if (ImGuiDraw::Vec1("Far Clip", farClip, .01f)) { camera.SetOrthographicFarClip(farClip); edited = true; }
    }

    ImGui::Separator();

    static const char *aspectModes[] = {"Match Viewport", "Ratio", "Pixels"};
    if (DrawEnumCombo("Aspect", component.AspectMode, aspectModes, 3)) edited = true;

    if (component.AspectMode == CameraAspectMode::Ratio) {
      struct Preset { const char *Name; float W, H; };
      static const Preset presets[] = {{"16:9", 16, 9}, {"16:10", 16, 10}, {"4:3", 4, 3},
                                       {"21:9", 21, 9}, {"3:2", 3, 2}, {"1:1", 1, 1}, {"9:16", 9, 16}};
      const char *current = "Custom";
      for (const auto &p : presets)
        if (p.W == component.AspectRatio.x && p.H == component.AspectRatio.y) current = p.Name;

      if (ImGui::BeginCombo("Ratio Preset", current)) {
        for (const auto &p : presets) {
          if (ImGui::Selectable(p.Name, p.W == component.AspectRatio.x && p.H == component.AspectRatio.y)) {
            component.AspectRatio = {p.W, p.H};
            edited = true;
          }
        }
        ImGui::EndCombo();
      }
      if (ImGuiDraw::Vec2("Ratio (W:H)", component.AspectRatio, 0.05f, 0.1f, 100.0f)) edited = true;
    }

    if (component.AspectMode == CameraAspectMode::Pixels) {
      if (ImGuiDraw::Vec2("Resolution", component.Resolution, 1.0f, 1.0f, 16384.0f)) {
        component.Resolution = glm::round(component.Resolution);
        edited = true;
      }
      if (ImGuiDraw::CheckBox("Integer Scaling", component.IntegerScaling)) edited = true;
      ImGui::TextDisabled("Aspect %.3f", component.Resolution.x / std::max(component.Resolution.y, 1.0f));
    }

    if (ImGuiDraw::Vec4("Viewport Rect", component.ViewportRect, 0.005f, 0.0f, 1.0f)) edited = true;

    ImGui::Separator();

    static const char *clearModes[] = {"Solid Color", "Don't Clear"};
    if (DrawEnumCombo("Clear", component.ClearMode, clearModes, 2)) edited = true;
    if (component.ClearMode == CameraClearMode::SolidColor)
      if (ImGuiDraw::ColorEdit4("Background", component.BackgroundColor)) edited = true;

    ImGui::TreePop();
  }
  FinishComponent(selection, component, edited, header, "Camera");
}

void DrawRigidbody2DComponent(Entity selection) {
  if (!selection.HasComponent<Rigidbody2DComponent>())
    return;

  auto &component = selection.GetComponent<Rigidbody2DComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "Rigidbody2D", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    static const char *bodyTypes[] = {"Static", "Kinematic", "Dynamic"};
    if (DrawEnumCombo("Body Type", component.Type, bodyTypes, 3))
      edited = true;

    if (ImGuiDraw::CheckBox("Fixed Rotation", component.FixedRotation))
      edited = true;

    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "Rigidbody2D");
}

void DrawBoxCollider2DComponent(Entity selection) {
  if (!selection.HasComponent<BoxCollider2DComponent>())
    return;

  auto &component = selection.GetComponent<BoxCollider2DComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "BoxCollider2D", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (ImGuiDraw::Vec2("Offset", component.Offset)) edited = true;
    if (ImGuiDraw::Vec2("Size", component.Size, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Rotation", component.Rotation, 0.025f, 0, 0)) edited = true;
    if (ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds)) edited = true;
    if (ImGuiDraw::CheckBox("IsTrigger", component.IsTrigger)) edited = true;
    if (ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold, 0.025f, 0.0001f, 100000.f)) edited = true;

    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "BoxCollider2D");
}

void DrawCircleCollider2DComponent(Entity selection) {
  if (!selection.HasComponent<CircleCollider2DComponent>())
    return;

  auto &component = selection.GetComponent<CircleCollider2DComponent>();
  bool edited = false;
  auto header = BeginComponent(selection, "CircleCollider2D", component, edited);

  if (header.Open) {
    ImGui::Spacing();

    if (ImGuiDraw::Vec2("Offset", component.Offset)) edited = true;
    if (ImGuiDraw::Vec1("Radius", component.Radius, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds)) edited = true;
    if (ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f, 100000.f)) edited = true;
    if (ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold, 0.025f, 0.0001f, 100000.f)) edited = true;

    ImGui::TreePop();
  }

  FinishComponent(selection, component, edited, header, "CircleCollider2D");
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

bool DrawRuntimeScriptComponents(const ScriptField &field,
                                 Ref<ScriptInstance> scriptInstance,
                                 Ref<Scene> context) {
  switch (field.Type) {
    case DataType::Short: {
      int16_t data = scriptInstance->GetFieldData<int16_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, (int &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::UShort: {
      uint16_t data = scriptInstance->GetFieldData<uint16_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, (int &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Int: {
      int32_t data = scriptInstance->GetFieldData<int32_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::UInt: {
      uint32_t data = scriptInstance->GetFieldData<uint32_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, (int &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Long: {
      int64_t data = scriptInstance->GetFieldData<int64_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, (int &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::ULong: {
      uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
      if (ImGuiDraw::Int1(field.Name, (int &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Float: {
      float data = scriptInstance->GetFieldData<float>(field.Name);
      if (ImGuiDraw::Vec1(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Double: {
      double data = scriptInstance->GetFieldData<double>(field.Name);
      if (ImGuiDraw::Vec1(field.Name, (float &)data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Bool: {
      bool data = scriptInstance->GetFieldData<bool>(field.Name);
      if (ImGuiDraw::CheckBox(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Vector2: {
      glm::vec2 data = scriptInstance->GetFieldData<glm::vec2>(field.Name);
      if (ImGuiDraw::Vec2(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Vector3: {
      glm::vec3 data = scriptInstance->GetFieldData<glm::vec3>(field.Name);
      if (ImGuiDraw::Vec3(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::Vector4: {
      glm::vec4 data = scriptInstance->GetFieldData<glm::vec4>(field.Name);
      if (ImGuiDraw::Vec4(field.Name, data)) {
        scriptInstance->SetFieldData(field.Name, data);
        return true;
      }
      break;
    }
    case DataType::DynamicVector: {
      // Retrieve vector data live from the running script instance buffer
      VectorBuffer data = scriptInstance->GetVectorFieldData(field.Name);
      uint64_t count = data.GetCount();
      bool modified = false;

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
    case DataType::ScriptComponent: {
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

  return false;
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
            interaction = true;
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
    default:
      break;
  }
  return interaction;
}

bool DrawEditTimeScriptComponents(const UUID &fieldID, const ScriptField &field,
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
        return true;
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
            if (elementPtr &&
              DrawEditTimeScriptVector(field, i, elementPtr, context,
                                       selection)) {
              getOrCreateInstance().SetVectorValue(data.Data);
              return true;
            }
          }

          ImGui::TreePop();
        }
      } else {
        ImGuiDraw::StaticString(field.Name, "(empty)");
      }

      float buttonWidth = 20.0f;
      float spacing = 4.0f;
      float totalWidth = (buttonWidth * 2.0f) + spacing;

      ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                           ImGui::GetContentRegionAvail().x - totalWidth);

      if (ImGui::Button("+", ImVec2(buttonWidth, 0))) {
        data.PushBackDefault(field.ElementSize);
        getOrCreateInstance().SetVectorValue(data.Data);
        return true;
      }

      ImGui::SameLine(0.0f, spacing);
      ImGui::BeginDisabled(count == 0);
      if (ImGui::Button("-", ImVec2(buttonWidth, 0))) {
        data.PopBack(field.ElementSize);
        getOrCreateInstance().SetVectorValue(data.Data);
        return true;
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
          return true;
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
          ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
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
            return true;
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
            return true;
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
              if (script.ClassName == field.TypeName) {
                isValidDragTarget = true;
                break;
            }
            }
          }
        }
      }

      if (isValidDragTarget) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              ImVec4(0.15f, 0.55f, 0.25f, 0.8f));
      }

      ImGuiDraw::DataField(field.Name, label);

      if (isValidDragTarget) {
        ImGui::PopStyleColor();
      }

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
                if (script.ClassName == field.TypeName) {
                  matchesScript = true;
                  break;
                }
              }
            }

            if (matchesScript) {
              getOrCreateInstance().SetValue(droppedEntityID);
              return true;
            }
          }
        }
        ImGui::EndDragDropTarget();
      }
      break;
    }
  }
  return false;
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

    const bool isChild = selection.GetComponent<RelationshipComponent>().Parent != 0;
    auto header = DrawComponentHeader("Script", typeid(ScriptComponentContainer).hash_code(),
                                      sc.PrefabDirty, sc.EditDirty, isChild);
    if (header.Reset)
      sc = ScriptComponent();
    if (header.Remove)
      scriptsToRemove.push_back(currentIdx);

    const bool open = header.Open;    ImGui::PopStyleColor(3);
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
          sc.ScriptID = sc.ClassName.empty() ? 0 : Hash::GenerateFNVHash(sc.ClassName);
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

      Ref<ScriptClass> scriptClass = scriptEngine.GetScriptClass(sc.ClassName.c_str());
      const auto &fields = scriptClass->GetFields();

      auto &scriptFields = scriptEngine.GetScriptFieldMap(selection);
      auto &fieldInstanceMap = scriptFields[sc.ScriptID];

      bool edited = false;
      for (const auto &[fieldID, field] : fields) {
        if (DrawEditTimeScriptComponents(fieldID, field, context, selection, fieldInstanceMap))
          edited = true;
      }
      ImGui::TreePop();
      if (edited) {
        component.EditDirty = true;
        sc.EditDirty = true;
        auto &rc = selection.GetComponent<RelationshipComponent>();
        Entity root = selection;
        if (rc.Parent != 0) {
          root = selection.GetRootParent();
        }

        if (root.HasComponent<PrefabLinkComponent>() && rc.Parent != 0)
          component.PrefabDirty = true;
      }
    }
    ImGui::PopID();
  }

  for (auto it = scriptsToRemove.rbegin(); it != scriptsToRemove.rend(); ++it) {
    auto &osc = selection.GetComponent<ObjectStateComponent>();
    osc.DeletedComponents.push_back(component.Scripts[*it].ClassName);
    selection.RemoveScriptComponent(*it);
  }
}

} // namespace CatEngine
