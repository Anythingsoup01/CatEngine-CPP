#include "cepch.h"
#include "SceneImporter.h"

#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/SceneSerializer.h"

namespace CatEngine
{
    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, Ref<Asset::MetaData>& metaData)
    {
        CE_PROFILE_FUNCTION();
        return LoadScene(Project::GetAssetDirectory() / metaData->FilePath);
    }

    Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const Ref<Asset::MetaData>& metaData)
    {
        CE_PROFILE_FUNCTION();
        return LoadScene(Project::GetAssetDirectory() / metaData->FilePath);
    }
    
    Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path &filePath)
    {
        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(filePath);

        return scene;
    }

}

