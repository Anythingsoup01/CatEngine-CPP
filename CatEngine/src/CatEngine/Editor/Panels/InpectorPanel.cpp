#include "cepch.h" // IWYU pragma: keep
#include "InpectorPanel.h"

#include "DrawComponents.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/SceneHierarchyPanel.h"
#include "CatEngine/Prefab/Prefab.h"
#include "CatEngine/Prefab/PrefabManager.h"
#include "CatEngine/Project/Project.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace CatEngine {

InspectorPanel::InspectorPanel(const Ref<Scene> &context)
    : m_Context(context) {}

void InspectorPanel::SetSceneContext(const Ref<Scene> &context) {
  m_EntitySelection = {};
  m_Context = context;
}

void InspectorPanel::OnImGuiRender(bool &isOpen) {
  ImGui::Begin("Inspector");
  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
    ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
    ImGui::FocusWindow(ImGui::GetCurrentWindow());
  }
  if (m_EntitySelection || m_AssetSelection) {
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    if (m_EntitySelection) {
      auto &stateComponent = m_EntitySelection.GetComponent<ObjectStateComponent>();
      if (stateComponent.Locked) {
        ImGui::BeginDisabled();
      }
      DrawComponents(m_EntitySelection);

      bool Components2DAvaliable =
          ((!m_EntitySelection.HasComponent<SpriteRendererComponent>() &&
            !m_EntitySelection.HasComponent<CircleRendererComponent>()) ||
           !m_EntitySelection.HasComponent<Rigidbody2DComponent>() ||
           (!m_EntitySelection.HasComponent<BoxCollider2DComponent>() &&
            !m_EntitySelection.HasComponent<CircleCollider2DComponent>()));

      float lineHeight =
          GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;

      float buttonSize = contentRegionAvailable.x / 2.f;
      ImGui::SetCursorPosX(buttonSize - (buttonSize / 2.f));
      if (ImGui::Button("Add Component",
                        ImVec2{contentRegionAvailable.x / 2.f, lineHeight}))
        ImGui::OpenPopup("AddComponent");

      if (ImGui::BeginPopup("AddComponent")) {
        DisplayAddComponentEntry<CameraComponent>("Camera Component");
        if (Components2DAvaliable) {
          bool opened = ImGui::TreeNodeEx("2D Components", treeNodeFlags);

          if (opened) {
            DisplayAddComponentEntries<SpriteRendererComponent,
                                       CircleRendererComponent>(
                "Sprite Renderer Component", "Circle Renderer Component");
            DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
            DisplayAddComponentEntries<BoxCollider2DComponent,
                                       CircleCollider2DComponent>(
                "Box Collider 2D", "Circle Collider 2D");
            ImGui::TreePop();
          }
        }
        DisplayAddScriptComponent("Script");
        ImGui::EndPopup();
      }

      if (stateComponent.Locked) {
        ImGui::EndDisabled();
      }

    } else if (m_AssetSelection) {
      DrawAsset(m_AssetSelection);
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         (((float)contentRegionAvailable.x -
                           ((float)contentRegionAvailable.x / 2.f)) *
                          .5f));

    ImGui::Dummy(ImVec2{ImGui::GetContentRegionAvail().x, 10});
  }
  ImGui::End();
}

void InspectorPanel::UpdateEntitySelection() {
  auto sceneHierarchy = PanelManager::Get().GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel");
  m_EntitySelection = sceneHierarchy->GetSelectedEntity();
  m_AssetSelection = 0;
}

void InspectorPanel::SetAssetSelectionContext(AssetHandle handle) {
  m_EntitySelection = {};
  m_AssetSelection = handle;
}

void DrawEntityOverrideTreeRecursive(Scene *scene, Entity current, bool isRoot, const std::unordered_map<UUID, std::vector<std::string>> &dirtyMap) {
  if (!scene || !current)
    return;

  const UUID entityUUID = current.GetUUID();
  auto it = dirtyMap.find(entityUUID);
  const bool hasDirtyChanges = (it != dirtyMap.end() && !it->second.empty());

  std::string entityName = "Unnamed Entity";
  if (current.HasComponent<NameComponent>()) {
    entityName = current.GetComponent<NameComponent>().Name;
  } else if (current.HasComponent<TagComponent>()) {
    entityName = current.GetComponent<TagComponent>().Tag;
  }

  // Check whether any descendant has dirty entries so parent nodes stay expandable to reach them
  bool hasDirtyDescendant = false;
  if (current.HasComponent<RelationshipComponent>()) {
    const auto &rel = current.GetComponent<RelationshipComponent>();
    for (UUID childUUID : rel.Children) {
      Entity child = scene->GetEntityByUUID(childUUID);
      if (child) {
        auto childIt = dirtyMap.find(child.GetUUID());
        if (childIt != dirtyMap.end() && !childIt->second.empty()) {
          hasDirtyDescendant = true;
          break;
        }
      }
    }
  }

  if (hasDirtyChanges || hasDirtyDescendant) {
    ImGui::PushID(static_cast<uint64_t>(entityUUID));

    const std::string headerTitle = isRoot ? (entityName + " (Root)") : entityName;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!hasDirtyChanges && !hasDirtyDescendant) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (ImGui::TreeNodeEx("##OverrideNode", flags, "%s", headerTitle.c_str())) {
      if (hasDirtyChanges) {
        for (const std::string &changeLabel : it->second) {
          ImGui::BulletText("%s", changeLabel.c_str());
        }
      }

      if (current.HasComponent<RelationshipComponent>()) {
        const auto &rel = current.GetComponent<RelationshipComponent>();
        for (UUID childUUID : rel.Children) {
          Entity child = scene->GetEntityByUUID(childUUID);
          if (child) {
            DrawEntityOverrideTreeRecursive(scene, child, /*isRoot=*/false, dirtyMap);
          }
        }
      }

      ImGui::TreePop();
    }

    ImGui::PopID();
  }
}

void InspectorPanel::DrawComponents(Entity selection) {
  auto &stateComponent = selection.GetComponent<ObjectStateComponent>();
  auto &nameComponent = selection.GetComponent<NameComponent>();
  auto &tagComponent = selection.GetComponent<TagComponent>();
  auto &layerComponent = selection.GetComponent<LayerComponent>();

  // 1) Top row: Active checkbox, Entity Name, and Lock toggle
  if (ImGui::BeginTable("##InspectorHeaderTop", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadInnerX)) {
    ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 24.0f);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed, 24.0f);

    ImGui::TableNextRow();

    // Active toggle (Unity top-left enable box)
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    bool activeState = stateComponent.SelfActive;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{2.0f, 2.0f});
    if (ImGui::Checkbox("##VISIBILITY_BUTTON", &activeState)) {
      m_Context->SetEntityActive(selection, activeState);
      stateComponent.EditDirty = true;
      if (selection.HasComponent<PrefabLinkComponent>())
        stateComponent.PrefabDirty = true;
    }


    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Active");
    }

    // Entity name field
    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();
    char nameBuffer[256] = {};
    std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", nameComponent.Name.c_str());

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.15f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.20f, 0.24f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.12f, 0.13f, 0.16f, 1.0f));
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##ENTITY_NAME", nameBuffer, sizeof(nameBuffer))) {
      nameComponent.Name = nameBuffer;
      nameComponent.EditDirty = true;
      if (selection.HasComponent<PrefabLinkComponent>())
        nameComponent.PrefabDirty = true;
    }
    ImGui::PopStyleColor(3);

    // Lock toggle on the right
    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    bool lockState = stateComponent.SelfLocked;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{2.0f, 2.0f});
    if (ImGui::Checkbox("##LOCK_BUTTON", &lockState)) {
      m_Context->SetEntityLocked(selection, lockState);
      stateComponent.EditDirty = true;
      if (selection.HasComponent<PrefabLinkComponent>())
        stateComponent.PrefabDirty = true;
    }

    ImGui::PopStyleVar();
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Locked");
    }

    ImGui::EndTable();
  }

  ImGui::Spacing();

  // 2) Second row: Tag and Layer side-by-side
  if (ImGui::BeginTable("##TAGLAYER", 2, ImGuiTableFlags_NoPadInnerX)) {
    ImGui::TableSetupColumn("TagCol", ImGuiTableColumnFlags_WidthStretch, 0.5f);
    ImGui::TableSetupColumn("LayerCol", ImGuiTableColumnFlags_WidthStretch, 0.5f);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("Tag");
    ImGui::SameLine(0.0f, 6.0f);
    char tagBuffer[256] = {};
    std::snprintf(tagBuffer, sizeof(tagBuffer), "%s", tagComponent.Tag.c_str());
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##T", tagBuffer, sizeof(tagBuffer))) {
      tagComponent.Tag = tagBuffer;
      tagComponent.EditDirty = true;
      if (selection.HasComponent<PrefabLinkComponent>())
        tagComponent.PrefabDirty = true;
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("Layer");
    ImGui::SameLine(0.0f, 6.0f);
    char layerBuffer[256] = {};
    std::snprintf(layerBuffer, sizeof(layerBuffer), "%s", layerComponent.Layer.c_str());
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##L", layerBuffer, sizeof(layerBuffer))) {
      layerComponent.Layer = layerBuffer;
      layerComponent.EditDirty = true;
      if (selection.HasComponent<PrefabLinkComponent>())
        layerComponent.PrefabDirty = true;
    }

    ImGui::EndTable();
  }

  // 3) Prefab bar skeleton (shown when entity has a PrefabLinkComponent)
  if (selection.HasComponent<PrefabLinkComponent>()) {
    auto &prefabLink = selection.GetComponent<PrefabLinkComponent>();
    auto assetManager = Project::GetActive()->GetEditorAssetManager();

    std::string prefabLabel = "Prefab";
    if (prefabLink.PrefabID != 0 && AssetManager::IsAssetHandleValid(prefabLink.PrefabID)) {
      const auto &prefabMeta = assetManager->GetMetaData(prefabLink.PrefabID);
      prefabLabel = prefabMeta.AssetName.empty() ? prefabMeta.FilePath.stem().string() : prefabMeta.AssetName;
    }

    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{8.0f, 5.0f});

    // ImVec2(0.0f, 0.0f) with AutoResizeY expands to full available width and wraps to content height
    if (ImGui::BeginChild("##PrefabInfoBar", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding)) {
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted(("Prefab: " + prefabLabel).c_str());

      if (ImGui::SmallButton("Overrides")) {
        ImGui::OpenPopup("PrefabOverridesPopup");
      }

      // Left-aligned action buttons
      if (ImGui::BeginPopup("PrefabOverridesPopup")) {
        auto dirtyMap = PrefabManager::Get().GetDirtyEntities(prefabLink.PrefabID, selection);

        const UUID deletedBucketKey{1};
        auto deletedIt = dirtyMap.find(deletedBucketKey);
        const bool hasDeletedEntries = (deletedIt != dirtyMap.end() && !deletedIt->second.empty());

        bool anyEntityChanges = false;
        for (const auto &[uuid, changes] : dirtyMap) {
          if (uuid != deletedBucketKey && !changes.empty()) {
            anyEntityChanges = true;
            break;
          }
        }

        ImGui::TextUnformatted("Prefab Overrides");
        ImGui::Separator();

        if (!anyEntityChanges && !hasDeletedEntries) {
          ImGui::TextDisabled("No overrides detected.");
        } else {
          if (anyEntityChanges) {
            DrawEntityOverrideTreeRecursive(m_Context.get(), selection, /*isRoot=*/true, dirtyMap);
          }

          if (hasDeletedEntries) {
            if (anyEntityChanges) {
              ImGui::Spacing();
            }
            ImGuiTreeNodeFlags delFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (ImGui::TreeNodeEx("Deleted Entities##PrefabDeletedGroup", delFlags)) {
              ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.45f, 0.45f, 1.0f));
              for (const std::string &deletedEntry : deletedIt->second) {
                ImGui::BulletText("%s", deletedEntry.c_str());
              }
              ImGui::PopStyleColor();
              ImGui::TreePop();
            }
          }

          ImGui::Separator();
          if (ImGui::Button("Apply All")) {
            PrefabManager::Get().ApplyOverridesToPrefab(prefabLink.PrefabID, selection);
          }
          ImGui::SameLine();
          if (ImGui::Button("Revert All")) {
            // TODO: Restore overridden components/deleted entities from prefab and clear dirty flags
          }
        }

        ImGui::EndPopup();
      }

      ImGui::SameLine();
      if (ImGui::SmallButton("Unpack")) {
        // TODO: Remove PrefabLinkComponent from this entity (and optionally children)
      }

      // Right-aligned "Open" button
      const char *openLabel = "Open";
      const float openWidth = ImGui::CalcTextSize(openLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      const float rightEdge = ImGui::GetWindowContentRegionMax().x;
      if (rightEdge - openWidth > ImGui::GetCursorPosX()) {
        ImGui::SameLine(rightEdge - openWidth);
      } else {
        ImGui::SameLine();
      }
      if (ImGui::SmallButton(openLabel)) {
        // TODO: Open prefab asset in prefab-editing mode (prefabLink.PrefabID)
      }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
  }

  ImGui::Spacing();
  ImGui::Separator();

  DrawComponentsHelper(selection);

  ImGui::Dummy(ImVec2{ImGui::GetContentRegionAvail().x, 50});
}

void InspectorPanel::DrawAsset(const AssetHandle &handle) {
  const auto &asset =
      Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);

  switch (asset->GetType()) {
    case AssetType::Scene: {
      DrawSceneAsset(asset, handle);
      break;
    }
    case AssetType::Texture2D: {
      DrawTexture2DAsset(asset, handle);
      break;
    }
    case AssetType::Prefab: {
      DrawPrefabAsset(asset, handle);
      break;
    }
    default:
      break;
  }
}

void InspectorPanel::DrawSceneAsset(const Ref<Asset> &asset, const AssetHandle &handle) {
  Ref<Scene> sceneAsset = (Ref<Scene> &)asset;

  auto assetManager = Project::GetActive()->GetEditorAssetManager();
  Asset::MetaData currentMetaData = assetManager->GetMetaData(handle);

  static AssetHandle s_TrackedSceneHandle = 0;
  static Asset::MetaData s_SceneMetaDataBuffer = currentMetaData;

  if (s_TrackedSceneHandle != handle || s_SceneMetaDataBuffer.FilePath != currentMetaData.FilePath) {
    s_TrackedSceneHandle = handle;
    s_SceneMetaDataBuffer = currentMetaData;
  }

  ImGui::SeparatorText("Scene Asset");
  ImGuiDraw::TextEdit("Name", s_SceneMetaDataBuffer.AssetName);

  if (!s_SceneMetaDataBuffer.SourceFilePath.empty()) {
    ImGuiDraw::StaticString("Source File", s_SceneMetaDataBuffer.SourceFilePath.generic_string());
  }

  bool isDirty = (s_SceneMetaDataBuffer.AssetName != currentMetaData.AssetName);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  float buttonWidth = 80.0f;
  float spacing = ImGui::GetStyle().ItemSpacing.x;
  float rightAlignX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (buttonWidth * 2.0f + spacing);
  if (rightAlignX > ImGui::GetCursorPosX()) {
    ImGui::SetCursorPosX(rightAlignX);
  }

  ImGui::BeginDisabled(!isDirty);
  if (ImGui::Button("Revert", ImVec2(buttonWidth, 0.0f))) {
    s_SceneMetaDataBuffer = currentMetaData;
  }
  ImGui::SameLine();
  if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f))) {
    const std::filesystem::path assetRoot = std::filesystem::absolute(Project::GetAssetDirectory());

    if (!s_SceneMetaDataBuffer.FilePath.empty()) {
      std::filesystem::path oldAssetAbs = std::filesystem::absolute(assetRoot / s_SceneMetaDataBuffer.FilePath);
      std::filesystem::path newAssetAbs = oldAssetAbs.parent_path() / (s_SceneMetaDataBuffer.AssetName + ".catsceneasset");

      if (oldAssetAbs != newAssetAbs && std::filesystem::exists(oldAssetAbs)) {
        std::filesystem::copy_file(oldAssetAbs, newAssetAbs, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(oldAssetAbs);
        s_SceneMetaDataBuffer.FilePath = std::filesystem::relative(newAssetAbs, assetRoot);
      }
    }

    if (!s_SceneMetaDataBuffer.SourceFilePath.empty()) {
      std::filesystem::path oldSourceAbs = std::filesystem::absolute(assetRoot / s_SceneMetaDataBuffer.SourceFilePath);
      std::string sourceExt = oldSourceAbs.extension().string();
      std::filesystem::path newSourceAbs = oldSourceAbs.parent_path() / (s_SceneMetaDataBuffer.AssetName + sourceExt);

      if (oldSourceAbs != newSourceAbs && std::filesystem::exists(oldSourceAbs)) {
        std::filesystem::copy_file(oldSourceAbs, newSourceAbs, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(oldSourceAbs);
        s_SceneMetaDataBuffer.SourceFilePath = std::filesystem::relative(newSourceAbs, assetRoot);
      }
    }

    assetManager->ReloadAssetWithNewMetaData(handle, s_SceneMetaDataBuffer);
    s_SceneMetaDataBuffer = assetManager->GetMetaData(handle);
  }
  ImGui::EndDisabled();
}

void InspectorPanel::DrawTexture2DAsset(const Ref<Asset> &asset, const AssetHandle &handle) {
  static std::vector<const char *> s_TextureFormats = {
    "R8",
    "RGB8",
    "RGBA8",
  };

  static std::vector<const char *> s_TextureFilters = {"Linear", "Nearest"};

  static std::vector<const char *> s_TextureWrapFilters = {
    "Repeat",
    "Mirrored Repeat",
    "Clamp To Edge",
    "Clamp To Border",
  };

  Ref<Texture2D> textureAsset = (Ref<Texture2D> &)asset;

  Asset::MetaData currentMetaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
  static AssetHandle s_TrackedHandle = 0;
  static Asset::MetaData s_MetaDataBuffer = currentMetaData;

  // Reset buffer when switching selected assets or if underlying metadata changed externally
  if (s_TrackedHandle != handle || s_MetaDataBuffer.FilePath != currentMetaData.FilePath) {
    s_TrackedHandle = handle;
    s_MetaDataBuffer = currentMetaData;
  }

  ImGui::SeparatorText("Texture Settings");

  ImGuiDraw::TextEdit("Name", s_MetaDataBuffer.AssetName);
  ImGuiDraw::DataField("Source Image", s_MetaDataBuffer.SourceFilePath.generic_string());

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
      const char *raw_str = static_cast<const char *>(payload->Data);
      size_t len = static_cast<size_t>(payload->DataSize);
      if (len > 0 && raw_str[len - 1] == '\0') {
        --len;
      }

      std::string_view sv(raw_str, len);
      unsigned long parsedID = 0;
      auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), parsedID);
      bool isAssetHandle = (ec == std::errc() && ptr == sv.data() + sv.size());

      if (!isAssetHandle && !sv.empty()) {
        std::filesystem::path droppedPath(sv);

        // If the payload is relative to the active asset browser directory, resolve it;
        // if already relative to AssetDirectory or absolute, resolve appropriately.
        std::filesystem::path candidatePath = droppedPath.is_absolute()
          ? droppedPath
          : (std::filesystem::exists(droppedPath)
              ? (droppedPath)
              : std::filesystem::absolute(droppedPath));

        std::string ext = candidatePath.extension().string();
        for (char &ch : ext) {
          ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }

        const bool isSupportedImage =
          ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
          ext == ".tga" || ext == ".bmp" || ext == ".psd";

        if (isSupportedImage && std::filesystem::exists(candidatePath)) {
          std::filesystem::path relSource = std::filesystem::relative(candidatePath, Project::GetAssetDirectory());
          if (!relSource.empty() && relSource.native().front() != '.') {
            s_MetaDataBuffer.SourceFilePath = relSource;
          } else {
            s_MetaDataBuffer.SourceFilePath = candidatePath;
          }
        }
      }
    }
    ImGui::EndDragDropTarget();
  }


  ImGuiDraw::Combo("Min Filter", (int &)s_MetaDataBuffer.TextureMinFilter, s_TextureFilters);
  ImGuiDraw::Combo("Mag Filter", (int &)s_MetaDataBuffer.TextureMagFilter, s_TextureFilters);
  ImGuiDraw::Combo("Wrap Mode", (int &)s_MetaDataBuffer.TextureWrap, s_TextureWrapFilters);
  ImGuiDraw::StaticString("Format", ImageFormatToString(s_MetaDataBuffer.TextureFormat));
 
  if (textureAsset) {
    uint32_t w = textureAsset->GetWidth();
    uint32_t h = textureAsset->GetHeight();
    ImGui::Spacing();
    ImGui::TextDisabled("%ux%u  |  %s", w, h, ImageFormatToString(s_MetaDataBuffer.TextureFormat).c_str());

    if (w > 0 && h > 0) {
      float availWidth = ImGui::GetContentRegionAvail().x;
      float previewSize = std::clamp(availWidth * 0.65f, 64.0f, 192.0f);
      float aspect = static_cast<float>(w) / static_cast<float>(h);
      ImVec2 imageSize = (aspect >= 1.0f)
        ? ImVec2(previewSize, previewSize / aspect)
        : ImVec2(previewSize * aspect, previewSize);

      // Center preview horizontally
      float cursorX = ImGui::GetCursorPosX() + (availWidth - imageSize.x) * 0.5f;
      if (cursorX > ImGui::GetCursorPosX()) {
        ImGui::SetCursorPosX(cursorX);
      }
      ImGui::Image((ImTextureID)(uint64_t)(textureAsset->GetRendererID()), imageSize, {0,1}, {1,0});
    }
  }

  // Dirty check against live metadata
  bool isDirty =
    (s_MetaDataBuffer.AssetName != currentMetaData.AssetName) ||
    (s_MetaDataBuffer.SourceFilePath != currentMetaData.SourceFilePath) ||
    (s_MetaDataBuffer.TextureMinFilter != currentMetaData.TextureMinFilter) ||
    (s_MetaDataBuffer.TextureMagFilter != currentMetaData.TextureMagFilter) ||
    (s_MetaDataBuffer.TextureWrap != currentMetaData.TextureWrap) ||
    (s_MetaDataBuffer.TextureFormat != currentMetaData.TextureFormat);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Unity-style Revert + Apply row aligned to the right
  float buttonWidth = 80.0f;
  float spacing = ImGui::GetStyle().ItemSpacing.x;
  float rightAlignX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (buttonWidth * 2.0f + spacing);
  if (rightAlignX > ImGui::GetCursorPosX()) {
    ImGui::SetCursorPosX(rightAlignX);
  }

  ImGui::BeginDisabled(!isDirty);
  if (ImGui::Button("Revert", ImVec2(buttonWidth, 0.0f))) {
    s_MetaDataBuffer = currentMetaData;
    isDirty = false;
  }
  ImGui::SameLine();
  if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f))) {
    std::filesystem::path oldAssetPath = std::filesystem::absolute(Project::GetAssetDirectory() / s_MetaDataBuffer.FilePath);
    std::string newAssetName = s_MetaDataBuffer.AssetName + ".cattextureasset";
    std::filesystem::path newAssetPath = std::filesystem::absolute(
      Project::GetAssetDirectory() / s_MetaDataBuffer.FilePath.parent_path() / newAssetName
    );

    if (newAssetPath != oldAssetPath) {
      std::filesystem::copy(oldAssetPath, newAssetPath, std::filesystem::copy_options::overwrite_existing);
      std::filesystem::remove(oldAssetPath);
      s_MetaDataBuffer.FilePath = std::filesystem::relative(newAssetPath, Project::GetAssetDirectory());
    }

    Project::GetActive()->GetEditorAssetManager()->ReloadAssetWithNewMetaData(handle, s_MetaDataBuffer);
    s_MetaDataBuffer = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
  }
  ImGui::EndDisabled();
}

void InspectorPanel::DrawPrefabAsset(const Ref<Asset> &asset, const AssetHandle &handle) {
  Ref<Prefab> prefabAsset = (Ref<Prefab> &)asset;

  auto assetManager = Project::GetActive()->GetEditorAssetManager();
  Asset::MetaData currentMetaData = assetManager->GetMetaData(handle);

  static AssetHandle s_TrackedPrefabHandle = 0;
  static Asset::MetaData s_PrefabMetaDataBuffer = currentMetaData;

  if (s_TrackedPrefabHandle != handle || s_PrefabMetaDataBuffer.FilePath != currentMetaData.FilePath) {
    s_TrackedPrefabHandle = handle;
    s_PrefabMetaDataBuffer = currentMetaData;
  }

  ImGui::SeparatorText("Prefab Asset");
  ImGuiDraw::TextEdit("Name", s_PrefabMetaDataBuffer.AssetName);

  if (!s_PrefabMetaDataBuffer.SourceFilePath.empty()) {
    ImGuiDraw::StaticString("Source File", s_PrefabMetaDataBuffer.SourceFilePath.generic_string());
  }

  bool isDirty = (s_PrefabMetaDataBuffer.AssetName != currentMetaData.AssetName);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  float buttonWidth = 80.0f;
  float spacing = ImGui::GetStyle().ItemSpacing.x;
  float rightAlignX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (buttonWidth * 2.0f + spacing);
  if (rightAlignX > ImGui::GetCursorPosX()) {
    ImGui::SetCursorPosX(rightAlignX);
  }

  ImGui::BeginDisabled(!isDirty);
  if (ImGui::Button("Revert", ImVec2(buttonWidth, 0.0f))) {
    s_PrefabMetaDataBuffer = currentMetaData;
  }
  ImGui::SameLine();
  if (ImGui::Button("Apply", ImVec2(buttonWidth, 0.0f))) {
    const std::filesystem::path assetRoot = std::filesystem::absolute(Project::GetAssetDirectory());

    if (!s_PrefabMetaDataBuffer.FilePath.empty()) {
      std::filesystem::path oldAssetAbs = std::filesystem::absolute(assetRoot / s_PrefabMetaDataBuffer.FilePath);
      std::filesystem::path newAssetAbs = oldAssetAbs.parent_path() / (s_PrefabMetaDataBuffer.AssetName + ".catprefabasset");

      if (oldAssetAbs != newAssetAbs && std::filesystem::exists(oldAssetAbs)) {
        std::filesystem::copy_file(oldAssetAbs, newAssetAbs, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(oldAssetAbs);
        s_PrefabMetaDataBuffer.FilePath = std::filesystem::relative(newAssetAbs, assetRoot);
      }
    }

    if (!s_PrefabMetaDataBuffer.SourceFilePath.empty()) {
      std::filesystem::path oldSourceAbs = std::filesystem::absolute(assetRoot / s_PrefabMetaDataBuffer.SourceFilePath);
      std::string sourceExt = oldSourceAbs.extension().string();
      std::filesystem::path newSourceAbs = oldSourceAbs.parent_path() / (s_PrefabMetaDataBuffer.AssetName + sourceExt);

      if (oldSourceAbs != newSourceAbs && std::filesystem::exists(oldSourceAbs)) {
        std::filesystem::copy_file(oldSourceAbs, newSourceAbs, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(oldSourceAbs);
        s_PrefabMetaDataBuffer.SourceFilePath = std::filesystem::relative(newSourceAbs, assetRoot);
      }
    }

    assetManager->ReloadAssetWithNewMetaData(handle, s_PrefabMetaDataBuffer);
    s_PrefabMetaDataBuffer = assetManager->GetMetaData(handle);
  }
  ImGui::EndDisabled();
}

template <typename T>
void InspectorPanel::DisplayAddComponentEntry(const std::string &entryName) {
  if (!m_EntitySelection.HasComponent<T>()) {
    if (ImGui::MenuItem(entryName.c_str())) {
      m_EntitySelection.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
}

template <typename T, typename O>
void InspectorPanel::DisplayAddComponentEntries(
    const std::string &entryOneName, const std::string &entryTwoName) {
    if (!m_EntitySelection.HasComponent<T>() &&
      !m_EntitySelection.HasComponent<O>()) {
    if (ImGui::MenuItem(entryOneName.c_str())) {
      m_EntitySelection.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
  if (!m_EntitySelection.HasComponent<O>() &&
      !m_EntitySelection.HasComponent<T>()) {
    if (ImGui::MenuItem(entryTwoName.c_str())) {
      m_EntitySelection.AddComponent<O>();
      ImGui::CloseCurrentPopup();
    }
  }
}

void InspectorPanel::DisplayAddScriptComponent(const std::string &name) {
  if (ImGui::MenuItem(name.c_str())) {
    m_EntitySelection.AddScriptComponent();
    ImGui::CloseCurrentPopup();
  }
}

} // namespace CatEngine
