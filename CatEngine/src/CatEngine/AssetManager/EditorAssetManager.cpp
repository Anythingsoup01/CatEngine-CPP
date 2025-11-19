#include "cepch.h"
#include "EditorAssetManager.h"

#include "AssetImporter.h"

#include "CatEngine/Project/Project.h"
#include <yaml-cpp/yaml.h>

#include "CatEngine/Core/Formatter.h"
#include "yaml-cpp/emittermanip.h"

namespace CatEngine
{

    EditorAssetManager::~EditorAssetManager()
    {
        m_AssetRegistry.clear();
        m_LoadedAssets.clear();
    }

    static std::unordered_map<std::filesystem::path, AssetType> s_AssetExtensionMap = 
    {
        { ".catscene", AssetType::Scene },
        { ".png", AssetType::Texture2D },
        { ".jpg", AssetType::Texture2D },
        { ".jpeg", AssetType::Texture2D },
    };

    static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& path)
    {
        if (!s_AssetExtensionMap.contains(path.extension()))
        {
            CE_API_WARN("Extension cannot '{}' be found!", path.extension().c_str());
            return AssetType::None;
        }

        return s_AssetExtensionMap[path.extension()];
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
    {
        // Check if handle is valid
        if (!IsAssetHandleValid(handle))
            return nullptr;
        // Check if asset needs load
        Ref<Asset> asset;
        if (IsAssetLoaded(handle))
        {
            asset = m_LoadedAssets[handle];
        }
        else
        {
            // Load Asset
            const Asset::MetaData& metaData = GetMetaData(handle);
            asset = AssetImporter::ImportAsset(handle, metaData);
            if (!asset) 
            {
                CE_API_ERROR("EditorAssetManager::GetAsset - Asset Import Failed!");
            } // import failed

            m_LoadedAssets[handle] = asset;
        }
        // Return Asset
        return asset;
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
    {
        return m_AssetRegistry.contains(handle);
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
    {
        return m_LoadedAssets.contains(handle);
    }

    AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
    {

        if (!IsAssetHandleValid(handle))
            return AssetType::None;

        return m_AssetRegistry.at(handle).Type;
    }

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filePath)
    {
        AssetHandle handle;
        Asset::MetaData metaData;

        metaData.FilePath = filePath;
        metaData.Type = GetAssetTypeFromFileExtension(filePath);

        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metaData);
        if (asset)
        {
            asset->m_Handle = handle;
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metaData;
            SerializeAssetRegistry();

            return handle;
        }
        return 0;
    }
    void EditorAssetManager::DeleteAsset(AssetHandle handle)
    {
        {
        auto it = m_AssetRegistry.find(handle);
        if (it != m_AssetRegistry.end())
            m_AssetRegistry.erase(it);
        }
        {
        auto it = m_LoadedAssets.find(handle);
        if (it != m_LoadedAssets.end())
            m_LoadedAssets.erase(it);
        }

        SerializeAssetRegistry();
    }

    const Asset::MetaData& EditorAssetManager::GetMetaData(AssetHandle handle) const
    {
        static Asset::MetaData s_NullMetaData;
        if (!IsAssetHandleValid(handle))
            return s_NullMetaData;

        return m_AssetRegistry.at(handle);
    }

    const std::filesystem::path& EditorAssetManager::GetFilePath(AssetHandle handle) const
    {
        return GetMetaData(handle).FilePath;
    }

    void EditorAssetManager::SerializeAssetRegistry()
    {
        auto path = Project::GetAssetRegistryPath();

        std::ofstream fout(path);
        if (!fout.is_open())
        {
            CE_API_ASSERT(false, "No Asset Registry Found!");
            return;
        }

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "AssetRegistry" << YAML::Value;

        out << YAML::BeginSeq;
        for (const auto& [handle, metaData] : m_AssetRegistry)
        {
            out << YAML::BeginMap; // Asset
            out << YAML::Key << "Handle" << YAML::Value << handle;
            std::string filePathStr = metaData.FilePath.generic_string();
            out << YAML::Key << "Name" << YAML::Value << metaData.AssetName;
            out << YAML::Key << "FilePath" << YAML::Value << filePathStr;
            out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metaData.Type);
            if (metaData.Type == AssetType::Texture2D)
            {
                out << YAML::Key << "MinFilter" << YAML::Value << TextureParameterToString(metaData.TextureMinFilter);
                out << YAML::Key << "MagFilter" << YAML::Value << TextureParameterToString(metaData.TextureMagFilter);
                out << YAML::Key << "WrapOption" << YAML::Value << TextureWrapParameterToString(metaData.TextureWrap);
                out << YAML::Key << "ImageFormat" << YAML::Value << ImageFormatToString(metaData.TextureFormat);
            }
            out << YAML::EndMap;
        }
        out <<  YAML::EndSeq;

        out << YAML::EndMap; // Root 

        fout << out.c_str();

    }

    bool EditorAssetManager::DeserializeAssetRegistry()
    {
        YAML::Node data = YAML::LoadFile(Project::GetAssetRegistryPath());

        auto registryNode = data["AssetRegistry"];
        if (!registryNode)
            return false;

        for (const auto& node : registryNode)
        {
            AssetHandle handle = node["Handle"].as<AssetHandle>();
            Asset::MetaData& metaData = m_AssetRegistry[handle];
            metaData.AssetName = node["Name"].as<std::string>();
            metaData.FilePath = node["FilePath"].as<std::string>();
            metaData.Type = StringToAssetType(node["Type"].as<std::string>());
            if (metaData.Type == AssetType::Texture2D)
            {
                metaData.TextureMinFilter = StringToTextureParameter(node["MinFilter"].as<std::string>());
                metaData.TextureMagFilter = StringToTextureParameter(node["MagFilter"].as<std::string>());
                metaData.TextureWrap = StringToTextureWrapParameter(node["WrapOption"].as<std::string>());
                metaData.TextureFormat = StringToImageFormat(node["ImageFormat"].as<std::string>());
            }
        }

        return true;

    }


}
