#include "cepch.h"
#include "Project.h"

#include "ProjectSerializer.h"

namespace CatEngine
{
    Project::Project()
    {
    }

    Ref<Project> Project::New()
    {
        s_ActiveProject = CreateRef<Project>();
        return s_ActiveProject;
    }
    
    Ref<Project> Project::Load(const std::filesystem::path& filePath)
    {
        ProjectSerializer serializer;
        if (serializer.Deserialize(filePath))
        {
            s_ActiveProject->m_ActiveProjectDirectory = filePath.parent_path();
            std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
            s_ActiveProject->m_AssetManager = editorAssetManager;
            editorAssetManager->DeserializeAssetRegistry();
            return s_ActiveProject;
        }

        return nullptr;
    }
    bool Project::Save(const std::filesystem::path& filePath)
    {
        ProjectSerializer serializer;
        if (serializer.Serialize(filePath))
        {
            s_ActiveProject->m_ActiveProjectDirectory = filePath.parent_path();
            const auto& editorAssetManager = s_ActiveProject->GetEditorAssetManager();
            editorAssetManager->SerializeAssetRegistry();
            return true;
        }

        return false;
    }
}
