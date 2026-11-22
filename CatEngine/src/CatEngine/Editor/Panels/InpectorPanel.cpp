#include "InpectorPanel.h"
#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Editor/Panels/DrawComponents.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Prefab/Prefab.h"
#include "CatEngine/Scene/Scene.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstdint>
#include <filesystem>

namespace CatEngine {

InspectorPanel::InspectorPanel(const Ref<Scene> &context)
    : m_Context(context) {}

void InspectorPanel::SetSceneContext(const Ref<Scene> &context) {
  m_EntitySelection = {};
  m_Context = context;
}

void InspectorPanel::OnImGuiRender(bool &isOpen) {
  ImGui::Begin("Inspector");
  if (m_EntitySelection || m_AssetSelection) {
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    if (m_EntitySelection) {
      auto &stateComponent = m_EntitySelection.GetComponent<ObjectStateComponent>();
      if (stateComponent.isLocked) {
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

      if (stateComponent.isLocked) {
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

void InspectorPanel::SetEntitySelectionContext(Entity e) {
  m_EntitySelection = e;
  m_AssetSelection = 0;
}

void InspectorPanel::SetAssetSelectionContext(AssetHandle handle) {
  m_EntitySelection = {};
  m_AssetSelection = handle;
}

void InspectorPanel::DrawComponents(Entity selection) {
  if (ImGui::BeginTable("##TAGLAYER", 4)) {
    ImGui::TableSetupColumn("ACTIVE_STATE", ImGuiTableColumnFlags_WidthFixed, 10);
    ImGui::TableSetupColumn("TAG", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("LAYER", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    auto &stateComponent = selection.GetComponent<ObjectStateComponent>();
    bool activeState = stateComponent.SelfActive;
    if (ImGui::Checkbox("##VISIBILITY_BUTTON", &activeState)) {
      m_Context->SetEntityActive(selection, activeState);
    }

    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Active");

    ImGui::TableSetColumnIndex(1);
    auto &tag = selection.GetComponent<TagComponent>().Tag;

    ImGui::PushItemWidth(-1.0f);
    char tagBuffer[256];
    memset(tagBuffer, 0, sizeof(tagBuffer));
    strncpy(tagBuffer, tag.c_str(), 256);
    ImGui::Text("Tag");
    ImGui::SameLine();
    if (ImGui::InputText("##T", tagBuffer, sizeof(tagBuffer))) {
      tag = std::string(tagBuffer);
    }
    ImGui::PopItemWidth();

    ImGui::TableSetColumnIndex(2);
    auto &layer = selection.GetComponent<LayerComponent>().Layer;

    ImGui::PushItemWidth(-1.0f);
    char layerBuffer[256];
    memset(layerBuffer, 0, sizeof(layerBuffer));
    strncpy(layerBuffer, layer.c_str(), 256);
    ImGui::Text("Layer");
    ImGui::SameLine();
    if (ImGui::InputText("##L", layerBuffer, sizeof(layerBuffer))) {
      layer = std::string(layerBuffer);
    }
    ImGui::PopItemWidth();

    ImGui::EndTable();
  }

  if (ImGui::BeginTable("##NAME", 1)) {
    ImGui::TableSetupColumn("NAME", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(-1.0f);
    auto &name = selection.GetComponent<NameComponent>().Name;

    char nameBuffer[256];
    memset(nameBuffer, 0, sizeof(nameBuffer));
    strncpy(nameBuffer, name.c_str(), 256);
    ImGui::Text("Name");
    ImGui::SameLine();
    if (ImGui::InputText("##N", nameBuffer, sizeof(nameBuffer))) {
      name = std::string(nameBuffer);
    }

    ImGui::PopItemWidth();

    ImGui::EndTable();
  }

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
