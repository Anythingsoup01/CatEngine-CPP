#include "cepch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "SceneImporter.h"

namespace CatEngine
{
    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const Asset::MetaData&)>;
    static std::unordered_map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
        { AssetType::Texture2D, TextureImporter::ImportTexture2D },
        { AssetType::Scene, SceneImporter::ImportScene }
    };

    Ref<Asset> AssetImporter::ImportAsset(const AssetHandle &handle, const Asset::MetaData &metaData)
    {
        if (!s_AssetImportFunctions.contains(metaData.Type))
        {
            CE_API_ERROR("No importer avaliable for AssetType: {}", (uint16_t)metaData.Type);
            return nullptr;
        }
        return s_AssetImportFunctions.at(metaData.Type)(handle, metaData);
    }
}
