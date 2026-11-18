#include "EditorAssetManager.h"
#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/AssetManager/Importers/SceneImporter.h"
#include "CatEngine/AssetManager/Importers/TextureImporter.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
#include "cepch.h"

#include "AssetImporter.h"

#include "CatEngine/Project/Project.h"
#include <cstdint>
#include <filesystem>
#include <yaml-cpp/yaml.h>

#include "CatEngine/Core/Formatter.h"
#include "yaml-cpp/emittermanip.h"

namespace CatEngine {

EditorAssetManager::~EditorAssetManager() {
  m_AssetRegistry.clear();
  m_LoadedAssets.clear();
}

static std::unordered_map<std::filesystem::path, AssetType>
    s_AssetExtensionMap = {
        {".catscene", AssetType::Scene},
        {".png", AssetType::Texture2D},
        {".jpg", AssetType::Texture2D},
        {".jpeg", AssetType::Texture2D},
        {".catprefab", AssetType::Prefab},
};

static AssetType
GetAssetTypeFromFileExtension(const std::filesystem::path &path) {
  if (s_AssetExtensionMap.find(path.extension()) == s_AssetExtensionMap.end()) {
    CE_API_WARN("Extension cannot '{}' be found!", path.extension().c_str());
    return AssetType::None;
  }

  return s_AssetExtensionMap[path.extension()];
}

Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) {
  // Check if handle is valid
  if (!IsAssetHandleValid(handle))
    return nullptr;
  // Check if asset needs load
  Ref<Asset> asset;
  if (IsAssetLoaded(handle)) {
    asset = m_LoadedAssets[handle];
  } else {
    // Load Asset
    const Asset::MetaData &metaData = GetMetaData(handle);
    asset = AssetImporter::ImportAsset(handle, metaData);
    if (!asset) {
      CE_API_ERROR("EditorAssetManager::GetAsset - Asset Import Failed!");
    } // import failed

    m_LoadedAssets[handle] = asset;
  }
  // Return Asset
  return asset;
}

bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const {
  return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const {
  return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
}

AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const {

  if (!IsAssetHandleValid(handle))
    return AssetType::None;

  return m_AssetRegistry.at(handle).Type;
}
AssetHandle EditorAssetManager::GetAssetHandle(const uint32_t &fileHash) const {
  if (!m_AssetRegistryFileMap.contains(fileHash))
    return {0};

  return m_AssetRegistryFileMap.at(fileHash);
}
Asset::MetaData EditorAssetManager::GetAssetMetaData(const uint32_t &fileHash) const {
  if (!m_AssetRegistryFileMap.contains(fileHash))
    return {};

  return m_AssetRegistry.at(m_AssetRegistryFileMap.at(fileHash));
}
AssetHandle EditorAssetManager::ImportNewAsset(const std::filesystem::path &filePath) {
  AssetHandle handle;
  Asset::MetaData metaData;

  metaData.SourceFilePath = filePath;
  metaData.Type = GetAssetTypeFromFileExtension(filePath);
  std::string fileName = filePath.filename().string();
  size_t extensionPos = fileName.find('.');
  fileName = fileName.erase(extensionPos);
  metaData.AssetName = fileName;

  std::filesystem::path assetPath = filePath;
  switch (metaData.Type) {
    case AssetType::Scene: {
      assetPath = assetPath.replace_extension(".catsceneasset");
      break;
    }
    case AssetType::Texture2D: {
      assetPath = assetPath.replace_extension(".cattextureasset");
      break;
    }
    case AssetType::Prefab: {
      assetPath = assetPath.replace_extension(".catprefabasset");
      break;
    }
    default:
      break;
  }

  metaData.FilePath = assetPath;

  Ref<Asset> asset = AssetImporter::ImportAsset(handle, metaData);
  if (asset) {
    asset->m_Handle = handle;
    m_LoadedAssets[handle] = asset;
    m_AssetRegistry[handle] = metaData;
    SerializeAsset(handle);
    RebuildAssetRegistry();
return handle;
  }
  return 0;

}

AssetHandle EditorAssetManager::ImportAssetFromDisk(const AssetHandle &handle, const Asset::MetaData &metaData) {
  if (!handle)
    return 0;

  Ref<Asset> asset = AssetImporter::ImportAsset(handle, metaData);
  if (asset) {
    asset->m_Handle = handle;
    m_LoadedAssets[handle] = asset;
    m_AssetRegistry[handle] = metaData;
    return handle;
  }
  return 0;
}

AssetHandle EditorAssetManager::ReloadAssetWithNewMetaData(
    const AssetHandle &handle, const Asset::MetaData &metaData) {
  if (!handle)
    return 0;

  DeleteAsset(handle);

  Ref<Asset> asset = AssetImporter::ImportAsset(handle, metaData);
  if (asset) {
    asset->m_Handle = handle;
    m_LoadedAssets[handle] = asset;
    m_AssetRegistry[handle] = metaData;
    SerializeAsset(handle);
    RebuildAssetRegistry();
    return handle;
  }
  return 0;
}
void EditorAssetManager::DeleteAsset(AssetHandle handle) {
  auto &metaData = GetMetaData(handle);
  std::filesystem::path absPath = std::filesystem::absolute(Project::GetAssetDirectory() / metaData.FilePath);
  if (std::filesystem::exists(absPath)) {
    std::filesystem::remove(absPath);
  }

  switch (metaData.Type) {
    case AssetType::Prefab:
    case AssetType::Scene: {
      std::filesystem::path absSourcePath = std::filesystem::absolute(Project::GetAssetDirectory() / metaData.SourceFilePath);
      if (std::filesystem::exists(absSourcePath))
        std::filesystem::remove(absSourcePath);
    }
    default:
      break;
  }

  for (auto it = m_AssetRegistryFileMap.begin(); it != m_AssetRegistryFileMap.end(); it++) {
    auto [hash, assetHandle] = *it;
    if (assetHandle == handle) {
      m_AssetRegistryFileMap.erase(it);
      break;
    }
  }

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
}

const Asset::MetaData &EditorAssetManager::GetMetaData(AssetHandle handle) const {
  static Asset::MetaData s_NullMetaData;
  if (!IsAssetHandleValid(handle))
    return s_NullMetaData;

  return m_AssetRegistry.at(handle);
}

const std::filesystem::path &EditorAssetManager::GetFilePath(AssetHandle handle) const {
  return GetMetaData(handle).FilePath;
}

const std::filesystem::path &EditorAssetManager::GetSourceFilePath(AssetHandle handle) const {
  return GetMetaData(handle).SourceFilePath;
}

bool EditorAssetManager::IsAssetExtension(const std::string &extension) {
  if (extension == ".catsceneasset") {
    return true;
  }
  if (extension == ".cattextureasset") {
    return true;
  }
  if (extension == ".catprefabasset") {
    return true;
  }

  return false;
}
void EditorAssetManager::SerializeAsset(const AssetHandle &handle) {
  const auto &metaData = m_AssetRegistry.at(handle);
  std::filesystem::path path = Project::GetAssetDirectory() / metaData.FilePath;
  std::ofstream fout(path);

  YAML::Emitter out;
  out << YAML::BeginMap; // Root Begin

  out << YAML::Key << "Handle" << YAML::Value << handle;
  out << YAML::Key << "Name" << YAML::Value << metaData.AssetName;
  out << YAML::Key << "FilePath" << YAML::Value << metaData.FilePath.generic_string();
  out << YAML::Key << "SourceFilePath" << YAML::Value << metaData.SourceFilePath.generic_string();
  out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metaData.Type);
  if (metaData.Type == AssetType::Texture2D) {
    out << YAML::Key << "MinFilter" << YAML::Value << TextureParameterToString(metaData.TextureMinFilter);
    out << YAML::Key << "MagFilter" << YAML::Value << TextureParameterToString(metaData.TextureMagFilter);
    out << YAML::Key << "WrapOption" << YAML::Value << TextureWrapParameterToString(metaData.TextureWrap);
    out << YAML::Key << "ImageFormat" << YAML::Value << ImageFormatToString(metaData.TextureFormat);
  }

  out << YAML::EndMap; // Root End
  fout << out.c_str();
  fout.close();
}

void EditorAssetManager::SerializeAssetRegistry() {
  for (const auto &[handle, metaData] : m_AssetRegistry) {
    std::filesystem::path path = Project::GetAssetDirectory() / metaData.FilePath;
    std::ofstream fout(path);

    YAML::Emitter out;
    out << YAML::BeginMap; // Root Begin

    out << YAML::Key << "Handle" << YAML::Value << handle;
    out << YAML::Key << "Name" << YAML::Value << metaData.AssetName;
    out << YAML::Key << "FilePath" << YAML::Value << metaData.FilePath.generic_string();
    out << YAML::Key << "SourceFilePath" << YAML::Value << metaData.SourceFilePath.generic_string();
    out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metaData.Type);
    if (metaData.Type == AssetType::Texture2D) {
      out << YAML::Key << "MinFilter" << YAML::Value << TextureParameterToString(metaData.TextureMinFilter);
      out << YAML::Key << "MagFilter" << YAML::Value << TextureParameterToString(metaData.TextureMagFilter);
      out << YAML::Key << "WrapOption" << YAML::Value << TextureWrapParameterToString(metaData.TextureWrap);
      out << YAML::Key << "ImageFormat" << YAML::Value << ImageFormatToString(metaData.TextureFormat);
    }

    out << YAML::EndMap; // Root End
    fout << out.c_str();
    fout.close();
  }
}

bool EditorAssetManager::DeserializeAssetRegistry() {
  std::vector<std::filesystem::path> assetPaths;
  for (const std::filesystem::directory_entry& entry :
      std::filesystem::recursive_directory_iterator(Project::GetAssetDirectory())) {
    if (!entry.is_regular_file() || !IsAssetExtension(entry.path().extension())) {
      continue;
    }
    assetPaths.push_back(entry.path());
  }

  for (auto &path : assetPaths) {
    YAML::Node data;
    try {
      data = YAML::LoadFile(path);
    } catch (YAML::ParserException ex) {
      Ref<ConsolePanel> console = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
      console->WriteError("Failed to load asset file: " + path.string());
      continue;
    }

    AssetHandle handle = data["Handle"].as<AssetHandle>();
    Asset::MetaData &metaData = m_AssetRegistry[handle];
    metaData.AssetName = data["Name"].as<std::string>();
    metaData.FilePath = data["FilePath"].as<std::string>();
    metaData.SourceFilePath = data["SourceFilePath"].as<std::string>();
    metaData.Type = StringToAssetType(data["Type"].as<std::string>());
    if (metaData.Type == AssetType::Texture2D) {
      metaData.TextureMinFilter = StringToTextureParameter(data["MinFilter"].as<std::string>());
      metaData.TextureMagFilter = StringToTextureParameter(data["MagFilter"].as<std::string>());
      metaData.TextureWrap = StringToTextureWrapParameter(data["WrapOption"].as<std::string>());
      metaData.TextureFormat = StringToImageFormat(data["ImageFormat"].as<std::string>());
    }

    uint32_t pathHash = Hash::GenerateFNVHash(path.generic_string());
    m_AssetRegistryFileMap[pathHash] = handle;
  }

  for (auto &[handle, metaData] : m_AssetRegistry) {
    ImportAssetFromDisk(handle, metaData);
  }

  return true;
}
void EditorAssetManager::RebuildAssetRegistry() {
  m_AssetRegistry.clear();
  m_AssetRegistryFileMap.clear();
  m_LoadedAssets.clear();

  DeserializeAssetRegistry();
}

} // namespace CatEngine
