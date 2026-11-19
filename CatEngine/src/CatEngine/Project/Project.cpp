#include "Project.h"
#include "cepch.h"

#include "ProjectSerializer.h"

namespace CatEngine {

Project::Project() {}

// TODO: When instantiating a new prefab, it loads two entities (probably the parent ID not being set before the recursion or the recursion taking place even with no children)
// TODO: Entities can't be deleted

Ref<Project> Project::New() {
  s_ActiveProject = CreateRef<Project>();
  return s_ActiveProject;
}

Ref<Project> Project::Load(const std::filesystem::path &filePath) {
  ProjectSerializer serializer;
  if (serializer.Deserialize(filePath)) {
    s_ActiveProject->m_ActiveProjectDirectory = filePath.parent_path();
    std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
    s_ActiveProject->m_AssetManager = editorAssetManager;
    editorAssetManager->Init();
    editorAssetManager->DeserializeAssetRegistry();
    return s_ActiveProject;
  }

  return nullptr;
}

bool Project::Save(const std::filesystem::path &filePath) {
  ProjectSerializer serializer;
  if (serializer.Serialize(filePath)) {
    s_ActiveProject->m_ActiveProjectDirectory = filePath.parent_path();
    const auto &editorAssetManager = s_ActiveProject->GetEditorAssetManager();
    editorAssetManager->SerializeAssetRegistry();
    editorAssetManager->RebuildAssetRegistry();
    return true;
  }

  return false;
}

void Project::UpdateAssetManager() { s_ActiveProject->m_AssetManager->Poll(); }

} // namespace CatEngine
