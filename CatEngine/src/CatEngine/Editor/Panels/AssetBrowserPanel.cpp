#include "cepch.h"
#include "AssetBrowserPanel.h"
#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Prefab/PrefabManager.h"
#include <cstring>
#include <filesystem>
#include <imgui.h>

#include "CatEngine/Core/Application.h"

#include "CatEngine/Editor/IconManager.h"

const size_t MAX_FILE_PATH_LEN = 4096;

#include "CatEngine/Project/Project.h"

namespace CatEngine {

AssetBrowserPanel::AssetBrowserPanel() {}

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
      Project::GetConfig().LastLoadedAssetPath = assetRoot;
    }
    MoveAssetTarget(assetRoot);

    // Render intermediate path segments as clickable buttons
    std::filesystem::path accumulatedPath = assetRoot;
    for (const auto &p : relativePath) {
      if (p.empty() || p == ".")
        continue;

      ImGui::PushID(accumulatedPath.c_str());
      accumulatedPath /= p;
      ImGui::SameLine();
      ImGui::Text("/");
      ImGui::SameLine();

      std::string segmentName = p.string();
      if (ImGui::Button(segmentName.c_str())) {
        m_CurrentDirectory = accumulatedPath;
        Project::GetConfig().LastLoadedAssetPath = accumulatedPath;
        ImGui::PopID();
        break; // Stop iteration if user clicked an ancestor
      }

      MoveAssetTarget(accumulatedPath);

      ImGui::PopID();
    }

    ImGui::Separator();
  }

  bool itemHovered = false;
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
        itemHovered = ImGui::IsItemHovered();
      } else {
        DrawRegularFile(directoryEntry.is_directory(), path.filename().generic_string());
        itemHovered = ImGui::IsItemHovered();
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
      PrefabManager::Get().CreateNewPrefabFromEntity(entity, m_CurrentDirectory);
    }
    ImGui::EndDragDropTarget();
  }

  if (!itemHovered) {
    if (ImGui::BeginPopupContextItem()) {
      ImGui::Separator();

      if (ImGui::MenuItem("Create SampleScene2D")) {
        Application::Get().CreateNewSampleScene(m_CurrentDirectory);
      }
      if (ImGui::MenuItem("Create EmptyScene2D")) {
        Application::Get().CreateNewEmptyScene(m_CurrentDirectory);
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Create Prefab")) {
        PrefabManager::Get().CreateNewPrefab(m_CurrentDirectory);
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Create Texture2D")) {
        TextureSpecification spec;
        spec.Width = 1;
        spec.Height = 1;
      }

      ImGui::EndPopup();
    }

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

  ImGuiChildFlags flags = ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX;
  if (ImGui::BeginChild(handle, ImVec2(0,0), flags)) {
    ImGui::ImageButton("##ICON",
                       (ImTextureID)(uint64_t)icon->GetRendererID(),
                       {m_ThumbnailSize, m_ThumbnailSize}, {0, 1}, {1, 0});

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->SetAssetSelectionContext(handle);
      if (asset->GetType() == AssetType::Scene) {
        Application::Get().OpenSceneFromHandle(handle);
      } else if (asset->GetType() == AssetType::Prefab) {
        Application::Get().OpenSceneFromHandle(handle);
      }
    }

    if (ImGui::BeginDragDropSource()) {
      UUID assetHandle = handle;
      std::string handleStr = std::to_string(assetHandle);
      ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", handleStr.c_str(), handleStr.size() + 1);
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
    ImGui::Text("%s", metaData.AssetName.c_str());

    if (assetDeleted) {
      Project::GetActive()->GetEditorAssetManager()->DeleteAsset(handle);
      PrefabManager::Get().RemovePrefab(handle);
    }
    ImGui::EndChild();
  }

  }

void AssetBrowserPanel::DrawRegularFile(const bool &isDirectory, const std::filesystem::path &fileName) {
  Ref<Texture2D> icon = isDirectory
    ? IconManager::GetIcon("DirectoryIcon")
    : IconManager::GetIcon("ScriptFileIcon");

  ImGuiChildFlags flags = ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX;
  if (ImGui::BeginChild(fileName.c_str(), ImVec2(0,0), flags)) {

    ImGui::ImageButton("##ICON",
                       (ImTextureID)(uint64_t)icon->GetRendererID(),
                       {m_ThumbnailSize, m_ThumbnailSize}, {0, 1}, {1, 0});

    if (ImGui::IsItemHovered()) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (isDirectory) {
          m_CurrentDirectory /= fileName;
          Project::GetConfig().LastLoadedAssetPath = m_CurrentDirectory;
        }
      }
    }

    if (isDirectory) {
      MoveAssetTarget(m_CurrentDirectory / fileName);
    } else {
      if (ImGui::BeginDragDropSource()) {
        std::string out = (m_CurrentDirectory / fileName).generic_string();
        ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", out.c_str(), out.size() + 1);
        ImGui::TextUnformatted(fileName.generic_string().c_str());
        ImGui::EndDragDropSource();
      }
    }

    ImGui::TextWrapped("%s", fileName.c_str());

    ImGui::EndChild();
  }
}

void AssetBrowserPanel::MoveAssetTarget(const std::filesystem::path &targetPath) {
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
      const std::filesystem::path assetRoot = std::filesystem::absolute(Project::GetAssetDirectory());
      const std::filesystem::path canonicalTarget = std::filesystem::weakly_canonical(targetPath);

      if (isAsset) {
        UUID assetID = static_cast<UUID>(value);
        const Asset::MetaData &metaData = assetManager->GetMetaData(assetID);
        Asset::MetaData newMetaData = metaData;

        std::filesystem::path oldAbsPath = std::filesystem::absolute(assetRoot / metaData.FilePath);
        if (std::filesystem::exists(oldAbsPath) &&
            std::filesystem::weakly_canonical(oldAbsPath.parent_path()) != canonicalTarget) {
          std::filesystem::path newAbsPath = ResolveUniqueFilePath(targetPath, metaData.FilePath.filename());
          std::filesystem::copy_file(oldAbsPath, newAbsPath);
          std::filesystem::remove(oldAbsPath);
          newMetaData.FilePath = std::filesystem::relative(newAbsPath, assetRoot);
          if (newMetaData.AssetName == metaData.FilePath.stem().string()) {
            newMetaData.AssetName = newAbsPath.stem().string();
          }
        }

        switch (metaData.Type) {
          case AssetType::Prefab:
          case AssetType::Scene: {
            if (!metaData.SourceFilePath.empty()) {
              std::filesystem::path currentSourcePath = std::filesystem::absolute(assetRoot / metaData.SourceFilePath);
              if (std::filesystem::exists(currentSourcePath) &&
                  std::filesystem::weakly_canonical(currentSourcePath.parent_path()) != canonicalTarget) {
                std::filesystem::path newSourcePath = ResolveUniqueFilePath(targetPath, metaData.SourceFilePath.filename());
                std::filesystem::copy_file(currentSourcePath, newSourcePath);
                std::filesystem::remove(currentSourcePath);
                newMetaData.SourceFilePath = std::filesystem::relative(newSourcePath, assetRoot);
              }
            }
            break;
          }
          default:
            break;
        }

        assetManager->ReloadAssetWithNewMetaData(assetID, newMetaData);
      } else if (!sv.empty()) {
        std::filesystem::path inFile(sv);
        std::filesystem::path fromPath = m_CurrentDirectory / inFile;
        if (!std::filesystem::exists(fromPath)) {
          fromPath = assetRoot / inFile;
        }

        if (std::filesystem::exists(fromPath) &&
            std::filesystem::weakly_canonical(fromPath.parent_path()) != canonicalTarget) {
          std::filesystem::path toPath = ResolveUniqueFilePath(targetPath, inFile.filename());
          std::filesystem::copy_file(fromPath, toPath);
          std::filesystem::remove(fromPath);
          assetManager->RebuildAssetRegistry();
        }
      }
    }
    ImGui::EndDragDropTarget();
  }
}
std::filesystem::path AssetBrowserPanel::ResolveUniqueFilePath(const std::filesystem::path &targetPath, const std::filesystem::path &fileName) {
  std::filesystem::path candidate = targetPath / fileName;
  if (!std::filesystem::exists(candidate)) {
    return candidate;
  }

  const std::string stem = fileName.stem().string();
  const std::string ext = fileName.extension().string();

  uint64_t counter = 1;
  while (std::filesystem::exists(candidate)) {
    candidate = targetPath / (stem + "_" + std::to_string(counter) + ext);
    ++counter;
  }
  return candidate;
}

} // namespace CatEngine
