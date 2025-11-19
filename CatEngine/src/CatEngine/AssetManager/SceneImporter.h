#pragma once

#include "Asset.h"

#include "CatEngine/Scene/Scene.h"

namespace CatEngine
{
    class SceneImporter
    {
    public:
        static Ref<Scene> ImportScene(AssetHandle handle, Asset::MetaData& metaData);
        static Ref<Scene> ConstImportScene(AssetHandle handle, const Asset::MetaData& metaData);
        static Ref<Scene> LoadScene(const std::filesystem::path& filePath);
    };
}
