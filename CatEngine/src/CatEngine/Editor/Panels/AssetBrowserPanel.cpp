#include "AssetBrowserPanel.h"
#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Scene/Prefab.h"
#include "CatEngine/Scene/PrefabSerializer.h"
#include "cepch.h"
#include <cstring>
#include <filesystem>
#include <imgui.h>

#include "CatEngine/Core/Application.h"

#include "CatEngine/Editor/IconManager.h"

const size_t MAX_FILE_PATH_LEN = 4096;

#include "CatEngine/Project/Project.h"

namespace CatEngine {

AssetBrowserPanel::AssetBrowserPanel()
    : m_CurrentDirectory(Project::GetAssetDirectory()) {
}

void AssetBrowserPanel::SetSceneContext(const Ref<Scene> &context) {
  m_SelectionContext = 0;
  m_SceneContext = context;
}

void AssetBrowserPanel::OnImGuiRender(bool &isOpen) {
  if (ImGui::Begin("Assets", &isOpen)) {
    std::filesystem::path assetRoot = Project::GetAssetDirectory();
    std::filesystem::path relativePath =
        std::filesystem::relative(m_CurrentDirectory, assetRoot);

    // Root button (Asset Root)
    if (ImGui::Button("Assets")) {
      m_CurrentDirectory = assetRoot;
    }

    // Render intermediate path segments as clickable buttons
    std::filesystem::path accumulatedPath = assetRoot;
    for (const auto &p : relativePath) {
      if (p.empty() || p == ".")
        continue;

      accumulatedPath /= p;
      ImGui::SameLine();
      ImGui::Text("/");
      ImGui::SameLine();

      std::string segmentName = p.string();
      if (ImGui::Button(segmentName.c_str())) {
        m_CurrentDirectory = accumulatedPath;
        break; // Stop iteration if user clicked an ancestor
      }
    }

    ImGui::Separator();
  }

  // ==========================================
  // 2. CHILD WINDOW FOR GRID VIEW
  // ==========================================
  ImGui::BeginChild("AssetBrowserGrid", ImVec2(0, 0), false,
                    ImGuiWindowFlags_None);
  {
    static float padding = 5.f;
    float cellSize = m_ThumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
      columnCount = 1;

    // 1. Begin columns layout for the grid
    ImGui::Columns(columnCount, "AssetColumns", false);

    for (auto &directoryEntry :
         std::filesystem::directory_iterator(m_CurrentDirectory)) {
      const auto &path = directoryEntry.path();
      std::string filenameString = path.filename().string();

      if (directoryEntry.path().extension() == ".h")
        continue;

      if (strncmp(filenameString.c_str(), ".", 1) == 0)
        continue;

      // Hide raw save files
      if (path.extension() == ".catscene" || path.extension() == ".catprefab")
        continue;

      ImGui::PushID(filenameString.c_str());
      if (path.extension() == ".catsceneasset" ||
          path.extension() == ".cattextureasset" ||
          path.extension() == ".catprefabasset") {
        std::filesystem::path fullPath = std::filesystem::absolute(path);
        uint32_t fileHash = Hash::GenerateFNVHash(fullPath.generic_string());
        AssetHandle handle = Project::GetActive()->GetAssetManager()->GetAssetHandle(fileHash);
        DrawAsset(handle);
      } else {
        DrawRegularFile(directoryEntry.is_directory(), path.filename().generic_string());
      }


      // Advance to the next grid column
      ImGui::NextColumn();
      ImGui::PopID();
    }
    ImGui::Columns(1);
  }
  ImGui::EndChild();

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
      ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
      UUID entityID = *(UUID *)payload->Data;
      Entity entity = m_SceneContext->GetEntityByUUID(entityID);

      if (entity) {
        std::filesystem::path targetDirectory = m_CurrentDirectory;
        std::string entityName = entity.GetName();
        std::filesystem::path prefabPath = targetDirectory / (entityName + ".catprefab");

        int counter = 0;
        while (std::filesystem::exists(prefabPath)) {
          prefabPath =
            targetDirectory /
            (entityName + "_" + std::to_string(++counter) + ".catprefab");
        }

        Ref<Prefab> newPrefab = CreateRef<Prefab>();
        PrefabSerializer serializer(newPrefab);
        serializer.Serialize(prefabPath, entity, m_SceneContext);

        std::filesystem::path relativePath = std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
        Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(relativePath);
      }
    }
    ImGui::EndDragDropTarget();
  }

  ImGui::End();
}

void AssetBrowserPanel::DrawAsset(const AssetHandle &handle) {
  auto asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);
  auto metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
  if (!asset)
    return;
  Ref<Texture2D> icon = IconManager::GetIcon("ScriptFileIcon");
  
  if (asset->GetType() == AssetType::Texture2D) {
    icon = (const Ref<Texture2D> &)asset;
  }

  ImGui::ImageButton("##ICON",
                     (ImTextureID)(uint64_t)icon->GetRendererID(),
                     {m_ThumbnailSize, m_ThumbnailSize}, {0, 1}, {1, 0});

  if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
    Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
    inspector->SetAssetSelectionContext(handle);
  }

  if (ImGui::BeginDragDropSource()) {
    UUID assetHandle = handle;
    ImGui::SetDragDropPayload("ASSET_BROWSER_ASSET", &assetHandle, sizeof(UUID));
    ImGui::TextUnformatted(metaData.AssetName.c_str());
    ImGui::EndDragDropSource();
  }

  bool assetDeleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete"))
      assetDeleted = true;

    ImGui::EndPopup();
  }


  // TODO: Allow users to edit the name
  ImGui::TextWrapped("%s", metaData.AssetName.c_str());

  if (assetDeleted) {
    Project::GetActive()->GetEditorAssetManager()->DeleteAsset(handle);
  }
}

void AssetBrowserPanel::DrawRegularFile(const bool &isDirectory, const std::string &fileName) {
  Ref<Texture2D> icon = isDirectory
    ? IconManager::GetIcon("DirectoryIcon")
    : IconManager::GetIcon("ScriptFileIcon");
  ImGui::ImageButton("##ICON",
                     (ImTextureID)(uint64_t)icon->GetRendererID(),
                     {m_ThumbnailSize, m_ThumbnailSize}, {0, 1}, {1, 0});

  if (ImGui::IsItemHovered()) {
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      if (isDirectory)
        m_CurrentDirectory /= fileName;
    }
  }

  if (isDirectory) {
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ASSET")) {
        UUID assetID = *(UUID *)payload->Data;
        const Asset::MetaData &metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(assetID);
        Asset::MetaData newMetaData = metaData;
        std::filesystem::path newAbsPath = m_CurrentDirectory / fileName / metaData.FilePath.filename();
        newMetaData.FilePath = std::filesystem::relative(newAbsPath, Project::GetAssetDirectory());
        switch(metaData.Type) {
          case AssetType::Prefab:
          case AssetType::Scene: {
            std::filesystem::path newSourcePath = m_CurrentDirectory / fileName / metaData.SourceFilePath.filename();
            std::filesystem::path currentSourcePath = std::filesystem::absolute(Project::GetAssetDirectory() / metaData.SourceFilePath);
            std::filesystem::copy_file(currentSourcePath, newSourcePath);
            std::filesystem::remove(currentSourcePath);
            newMetaData.SourceFilePath = std::filesystem::relative(newSourcePath, Project::GetAssetDirectory());
            break;
          }
          default:
            break;
        }
        Project::GetActive()->GetEditorAssetManager()->ReloadAssetWithNewMetaData(assetID, newMetaData);

      }
      ImGui::EndDragDropTarget();
    }
  }


  ImGui::TextWrapped("%s", fileName.c_str());
}

} // namespace CatEngine
