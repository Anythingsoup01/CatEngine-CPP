#include "AssetBrowserPanel.h"
#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Scene/Prefab.h"
#include "CatEngine/Scene/PrefabSerializer.h"
#include "cepch.h"
#include <cstring>
#include <imgui.h>

#include "CatEngine/Core/Application.h"

#include "CatEngine/Editor/IconManager.h"

const size_t MAX_FILE_PATH_LEN = 4096;

#include "CatEngine/Project/Project.h"

namespace CatEngine {

AssetBrowserPanel::AssetBrowserPanel()
    : m_CurrentDirectory(Project::GetAssetDirectory()) {
  // TODO: Generate these as assets
  m_TreeNodes.push_back(TreeNode(".", 0));
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
    static float thumbnailSize = 75.f;
    float cellSize = thumbnailSize + padding;

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

      ImGui::PushID(filenameString.c_str());
      if (path.extension() == ".catsceneasset" ||
          path.extension() == ".cattextureasset" ||
          path.extension() == ".catprefabasset") {
        std::filesystem::path fullPath = std::filesystem::absolute(path);
        uint32_t fileHash = Hash::GenerateFNVHash(fullPath.generic_string());
        AssetHandle handle = Project::GetActive()->GetAssetManager()->GetAssetHandle(fileHash);
        Asset::MetaData metaData = Project::GetActive()->GetAssetManager()->GetAssetMetaData(fileHash);

        Ref<Texture2D> icon = IconManager::GetIcon("ScriptFileIcon");
        if (metaData.Type == AssetType::Texture2D) {
          const auto &asset =
              Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);
          icon = (const Ref<Texture2D> &)asset;
        }

        std::string fileName = filenameString;
        size_t extensionLen = path.extension().string().length();
        fileName.erase(fileName.length() - extensionLen);
        filenameString = fileName;

        ImGui::ImageButton("##ICON",
                           (ImTextureID)(uint64_t)icon->GetRendererID(),
                           {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InpectorPanel");
          inspector->SetAssetSelectionContext(handle);
        }
      } else {
        Ref<Texture2D> icon = directoryEntry.is_directory()
                                  ? IconManager::GetIcon("DirectoryIcon")
                                  : IconManager::GetIcon("ScriptFileIcon");
        ImGui::ImageButton("##ICON",
                           (ImTextureID)(uint64_t)icon->GetRendererID(),
                           {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

        if (ImGui::IsItemHovered()) {
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (directoryEntry.is_directory())
              m_CurrentDirectory /= path.filename();
          }
        }
      }


      ImGui::TextWrapped("%s", filenameString.c_str());

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
        std::filesystem::path prefabPath =
          targetDirectory / (entityName + ".catprefab");

        int counter = 0;
        while (std::filesystem::exists(prefabPath)) {
          prefabPath =
            targetDirectory /
            (entityName + "_" + std::to_string(++counter) + ".catprefab");
        }

        Ref<Prefab> newPrefab = CreateRef<Prefab>();
        PrefabSerializer serializer(newPrefab);
        serializer.Serialize(prefabPath, entity);

        std::filesystem::path relativePath =
          std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
      Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);

        RefreshAssetTree();
      }
    }
    ImGui::EndDragDropTarget();
  }

  ImGui::End();
}

void AssetBrowserPanel::RefreshAssetTree() {
  const auto &assetRegistry =
      Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();

  for (const auto &[handle, metaData] : assetRegistry) {
    uint32_t currentNodeIndex = 0;
    for (const auto &p : metaData.FilePath) {
      auto it = m_TreeNodes[currentNodeIndex].Children.find(p.generic_string());
      if (it != m_TreeNodes[currentNodeIndex].Children.end()) {
        currentNodeIndex = it->second;
      } else {
        TreeNode newNode(p, handle);
        newNode.Parent = currentNodeIndex;
        m_TreeNodes.push_back(newNode);

        m_TreeNodes[currentNodeIndex].Children[p] = m_TreeNodes.size() - 1;
        currentNodeIndex = m_TreeNodes.size() - 1;
      }
    }
  }
}

void AssetBrowserPanel::DrawAsset(const AssetHandle &handle) {}

void AssetBrowserPanel::ResetAssetDirectory() {
  m_CurrentDirectory = Project::GetAssetDirectory();
  RefreshAssetTree();
}
} // namespace CatEngine
