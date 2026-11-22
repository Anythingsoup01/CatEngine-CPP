#include "cepch.h" // IWYU pragma: keep

#include "AssetBrowserPanel.h"

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/Application.h"
#include "CatEngine/Editor/IconManager.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Prefab/PrefabManager.h"
#include "CatEngine/Project/Project.h"

#include <imgui.h>

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
        AssetHandle handle =
            Project::GetActive()->GetAssetManager()->GetAssetHandle(fileHash);
        DrawAsset(handle);
        itemHovered = ImGui::IsItemHovered();
      } else {
        DrawRegularFile(directoryEntry.is_directory(),
                        path.filename().generic_string());
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
      PrefabManager::Get().CreateNewPrefabFromEntity(entity,
                                                     m_CurrentDirectory);
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
  auto assetManager = Project::GetActive()->GetEditorAssetManager();
  auto asset = assetManager->GetAsset(handle);
  auto metaData = assetManager->GetMetaData(handle);
  if (!asset)
    return;

  Ref<Texture2D> icon = IconManager::GetIcon("ScriptFileIcon");
  if (asset->GetType() == AssetType::Texture2D) {
    icon = (const Ref<Texture2D> &)asset;
  }

  // Per-asset inline rename state keyed by handle
  static AssetHandle s_RenamingHandle = 0;
  static char s_RenameBuffer[256] = {};
  static bool s_FocusRenameInput = false;

  bool assetDeleted = false;
  bool triggerRename = false;

  ImGuiChildFlags flags =
      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX;
  if (ImGui::BeginChild(handle, ImVec2(0, 0), flags)) {
    ImGui::ImageButton("##ICON", (ImTextureID)(uint64_t)icon->GetRendererID(),
                       {m_ThumbnailSize, m_ThumbnailSize}, {0, 1}, {1, 0});

    if (ImGui::IsItemHovered() &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      Ref<InspectorPanel> inspector =
          PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->SetAssetSelectionContext(handle);
      if (asset->GetType() == AssetType::Scene ||
          asset->GetType() == AssetType::Prefab) {
        Application::Get().OpenSceneFromHandle(handle);
      }
    }

    if (ImGui::BeginDragDropSource()) {
      UUID assetHandle = handle;
      std::string handleStr = std::to_string(assetHandle);
      ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", handleStr.c_str(),
                                handleStr.size() + 1);
      ImGui::TextUnformatted(metaData.AssetName.c_str());
      ImGui::EndDragDropSource();
    }

    if (ImGui::BeginPopupContextItem("AssetContextMenu")) {
      if (ImGui::MenuItem("Properties")) {
        Ref<InspectorPanel> inspector =
            PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
        inspector->SetAssetSelectionContext(handle);
        ImGui::CloseCurrentPopup();
      }

      if (asset->GetType() == AssetType::Scene ||
          asset->GetType() == AssetType::Prefab) {
        if (ImGui::MenuItem("Open")) {
          Application::Get().OpenSceneFromHandle(handle);
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Rename")) {
        triggerRename = true;
        ImGui::CloseCurrentPopup();
      }

      // Type-specific actions
      if (asset->GetType() == AssetType::Texture2D) {
        ImGui::Separator();
        if (ImGui::MenuItem("Create Texture Atlas")) {
          // TODO: Create a new texture atlas asset referencing this Texture2D
          // `handle` e.g. CreateTextureAtlasFromTexture(handle, metaData);
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Delete")) {
        assetDeleted = true;
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }

    if (triggerRename) {
      s_RenamingHandle = handle;
      std::snprintf(s_RenameBuffer, sizeof(s_RenameBuffer), "%s",
                    metaData.AssetName.c_str());
      s_FocusRenameInput = true;
    }

    if (s_RenamingHandle == handle) {
      ImGui::SetNextItemWidth(m_ThumbnailSize);
      if (s_FocusRenameInput) {
        ImGui::SetKeyboardFocusHere();
        s_FocusRenameInput = false;
      }

      ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue |
                                       ImGuiInputTextFlags_AutoSelectAll;
      bool submitted = ImGui::InputText("##RenameAssetInput", s_RenameBuffer,
                                        sizeof(s_RenameBuffer), inputFlags);

      if (submitted || (!ImGui::IsItemActive() && ImGui::IsItemDeactivated())) {
        std::string newName(s_RenameBuffer);
        if (!newName.empty() && newName != metaData.AssetName) {
          const std::filesystem::path assetRoot =
              std::filesystem::absolute(Project::GetAssetDirectory());
          Asset::MetaData updatedMeta = metaData;
          updatedMeta.AssetName = newName;

          if (!updatedMeta.FilePath.empty()) {
            std::filesystem::path oldAssetAbs =
                std::filesystem::absolute(assetRoot / updatedMeta.FilePath);
            std::filesystem::path newAssetAbs =
                oldAssetAbs.parent_path() /
                (newName + oldAssetAbs.extension().string());
            if (oldAssetAbs != newAssetAbs &&
                std::filesystem::exists(oldAssetAbs)) {
              std::filesystem::copy_file(
                  oldAssetAbs, newAssetAbs,
                  std::filesystem::copy_options::overwrite_existing);
              std::filesystem::remove(oldAssetAbs);
              updatedMeta.FilePath =
                  std::filesystem::relative(newAssetAbs, assetRoot);
            }
          }

          if (!updatedMeta.SourceFilePath.empty()) {
            std::filesystem::path oldSourceAbs = std::filesystem::absolute(
                assetRoot / updatedMeta.SourceFilePath);
            std::filesystem::path newSourceAbs =
                oldSourceAbs.parent_path() /
                (newName + oldSourceAbs.extension().string());
            if (oldSourceAbs != newSourceAbs &&
                std::filesystem::exists(oldSourceAbs)) {
              std::filesystem::copy_file(
                  oldSourceAbs, newSourceAbs,
                  std::filesystem::copy_options::overwrite_existing);
              std::filesystem::remove(oldSourceAbs);
              updatedMeta.SourceFilePath =
                  std::filesystem::relative(newSourceAbs, assetRoot);
            }
          }

          assetManager->ReloadAssetWithNewMetaData(handle, updatedMeta);
        }
        s_RenamingHandle = 0;
      }
    } else {
      ImGui::Text("%s", metaData.AssetName.c_str());
    }

    if (assetDeleted) {
      assetManager->DeleteAsset(handle);
      PrefabManager::Get().RemovePrefab(handle);
    }

    ImGui::EndChild();
  }
}

void AssetBrowserPanel::DrawRegularFile(const bool &isDirectory,
                                        const std::filesystem::path &fileName) {
  Ref<Texture2D> icon = isDirectory ? IconManager::GetIcon("DirectoryIcon")
                                    : IconManager::GetIcon("ScriptFileIcon");

  std::string ext = fileName.extension().string();
  for (char &ch : ext) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }

  const bool isSupportedImage =
      !isDirectory && (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                       ext == ".tga" || ext == ".bmp" || ext == ".psd");

  static std::string s_RenamingEntryPath;
  static char s_RenameBuffer[256] = {};
  static bool s_FocusRenameInput = false;

  const std::filesystem::path fullPath = m_CurrentDirectory / fileName;
  const std::string entryKey = fullPath.generic_string();

  bool triggerRename = false;
  bool deleteEntry = false;
  bool createTexture2D = false;

  ImGuiChildFlags flags =
      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX;
  if (ImGui::BeginChild(fileName.c_str(), ImVec2(0, 0), flags)) {
    ImGui::ImageButton(
        "##ICON", (ImTextureID)(uint64_t)icon->GetRendererID(),
        {m_ThumbnailSize, ImGui::GetTextLineHeight() * 0 + m_ThumbnailSize},
        {0, 1}, {1, 0});

    if (ImGui::IsItemHovered()) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (isDirectory) {
          m_CurrentDirectory /= fileName;
          Project::GetConfig().LastLoadedAssetPath = m_CurrentDirectory;
        }
      }
    }

    if (isDirectory) {
      MoveAssetTarget(fullPath);
    } else {
      if (ImGui::BeginDragDropSource()) {
        std::string out = fullPath.generic_string();
        ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", out.c_str(),
                                  out.size() + 1);
        ImGui::TextUnformatted(fileName.generic_string().c_str());
        ImGui::EndDragDropSource();
      }
    }

    if (ImGui::BeginPopupContextItem("RegularFileContextMenu")) {
      if (isDirectory) {
        if (ImGui::MenuItem("Open")) {
          m_CurrentDirectory /= fileName;
          Project::GetConfig().LastLoadedAssetPath = m_CurrentDirectory;
          ImGui::CloseCurrentPopup();
        }
        ImGui::Separator();
      }

      if (ImGui::MenuItem("Rename")) {
        triggerRename = true;
        ImGui::CloseCurrentPopup();
      }

      if (isSupportedImage) {
        ImGui::Separator();
        if (ImGui::MenuItem("Create Texture2D")) {
          createTexture2D = true;
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Delete")) {
        deleteEntry = true;
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup();
    }

    if (triggerRename) {
      s_RenamingEntryPath = entryKey;
      // Preserve extension for files by default; directories edit their full
      // folder name
      std::string initialName =
          isDirectory ? fileName.filename().string() : fileName.stem().string();
      std::snprintf(s_RenameBuffer, sizeof(s_RenameBuffer), "%s",
                    initialName.c_str());
      s_FocusRenameInput = true;
    }

    if (s_RenamingEntryPath == entryKey) {
      ImGui::SetNextItemWidth(m_ThumbnailSize);
      if (s_FocusRenameInput) {
        ImGui::SetKeyboardFocusHere();
        s_FocusRenameInput = false;
      }

      ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue |
                                       ImGuiInputTextFlags_AutoSelectAll;
      bool submitted =
          ImGui::InputText("##RenameRegularFileInput", s_RenameBuffer,
                           sizeof(s_RenameBuffer), inputFlags);

      if (submitted || (!ImGui::IsItemActive() && ImGui::IsItemDeactivated())) {
        std::string enteredName(s_RenameBuffer);
        if (!enteredName.empty()) {
          std::filesystem::path targetName =
              isDirectory ? std::filesystem::path(enteredName)
                          : std::filesystem::path(
                                enteredName + fileName.extension().string());

          std::filesystem::path oldAbs = std::filesystem::absolute(fullPath);
          std::filesystem::path newAbs =
              std::filesystem::absolute(m_CurrentDirectory / targetName);

          if (oldAbs != newAbs && std::filesystem::exists(oldAbs)) {
            std::filesystem::copy(
                oldAbs, newAbs,
                isDirectory
                    ? std::filesystem::copy_options::recursive |
                          std::filesystem::copy_options::overwrite_existing
                    : std::filesystem::copy_options::overwrite_existing);
            std::filesystem::remove_all(oldAbs);
            Project::GetActive()
                ->GetEditorAssetManager()
                ->RebuildAssetRegistry();
          }
        }
        s_RenamingEntryPath.clear();
      }
    } else {
      ImGui::TextWrapped("%s", fileName.c_str());
    }

    if (createTexture2D) {
      std::filesystem::path absPath = std::filesystem::absolute(fullPath);
      Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(absPath);
    }

    if (deleteEntry) {
      std::filesystem::path absTarget = std::filesystem::absolute(fullPath);
      if (std::filesystem::exists(absTarget)) {
        std::filesystem::remove_all(absTarget);
        Project::GetActive()->GetEditorAssetManager()->RebuildAssetRegistry();
      }
    }

    ImGui::EndChild();
  }
}
void AssetBrowserPanel::MoveAssetTarget(
    const std::filesystem::path &targetPath) {
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
      const std::filesystem::path assetRoot =
          std::filesystem::absolute(Project::GetAssetDirectory());
      const std::filesystem::path canonicalTarget =
          std::filesystem::weakly_canonical(targetPath);

      if (isAsset) {
        UUID assetID = static_cast<UUID>(value);
        const Asset::MetaData &metaData = assetManager->GetMetaData(assetID);
        Asset::MetaData newMetaData = metaData;

        std::filesystem::path oldAbsPath =
            std::filesystem::absolute(assetRoot / metaData.FilePath);
        if (std::filesystem::exists(oldAbsPath) &&
            std::filesystem::weakly_canonical(oldAbsPath.parent_path()) !=
                canonicalTarget) {
          std::filesystem::path newAbsPath =
              ResolveUniqueFilePath(targetPath, metaData.FilePath.filename());
          std::filesystem::copy_file(oldAbsPath, newAbsPath);
          std::filesystem::remove(oldAbsPath);
          newMetaData.FilePath =
              std::filesystem::relative(newAbsPath, assetRoot);
          if (newMetaData.AssetName == metaData.FilePath.stem().string()) {
            newMetaData.AssetName = newAbsPath.stem().string();
          }
        }

        switch (metaData.Type) {
        case AssetType::Prefab:
        case AssetType::Scene: {
          if (!metaData.SourceFilePath.empty()) {
            std::filesystem::path currentSourcePath =
                std::filesystem::absolute(assetRoot / metaData.SourceFilePath);
            if (std::filesystem::exists(currentSourcePath) &&
                std::filesystem::weakly_canonical(
                    currentSourcePath.parent_path()) != canonicalTarget) {
              std::filesystem::path newSourcePath = ResolveUniqueFilePath(
                  targetPath, metaData.SourceFilePath.filename());
              std::filesystem::copy_file(currentSourcePath, newSourcePath);
              std::filesystem::remove(currentSourcePath);
              newMetaData.SourceFilePath =
                  std::filesystem::relative(newSourcePath, assetRoot);
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
            std::filesystem::weakly_canonical(fromPath.parent_path()) !=
                canonicalTarget) {
          std::filesystem::path toPath =
              ResolveUniqueFilePath(targetPath, inFile.filename());
          std::filesystem::copy_file(fromPath, toPath);
          std::filesystem::remove(fromPath);
          assetManager->RebuildAssetRegistry();
        }
      }
    }
    ImGui::EndDragDropTarget();
  }
}
std::filesystem::path AssetBrowserPanel::ResolveUniqueFilePath(
    const std::filesystem::path &targetPath,
    const std::filesystem::path &fileName) {
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
