#include "cepch.h" // IWYU pragma: keep

#include "ProjectSerializer.h"

#include "Project.h"

#include "CatEngine/Core/Formatter.h"

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

namespace CatEngine {

bool ProjectSerializer::Serialize(const std::filesystem::path &filePath) {
  CE_PROFILE_FUNCTION();

  const auto &config = Project::GetConfig();

  YAML::Emitter out;
  out << YAML::BeginMap; // Root
  out << YAML::Key << "Project" << YAML::Value;
  out << YAML::BeginMap; // Project
  out << YAML::Key << "Name" << YAML::Value << config.Name;
  out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
  out << YAML::Key << "LastLoadedAssetPath" << YAML::Value << config.LastLoadedAssetPath.string();
  out << YAML::Key << "OpenScenes" << YAML::Value;
  out << YAML::BeginSeq;
  for (auto &sceneHandle : config.OpenScenes) {
    out << YAML::BeginMap;
    out << YAML::Key << "AssetHandle" << YAML::Value << sceneHandle;
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(filePath);
  fout << out.c_str();

  return true;
}

bool ProjectSerializer::Deserialize(const std::filesystem::path &filePath) {
  auto &config = Project::GetConfig();

  YAML::Node data;
  try {
    data = YAML::LoadFile(filePath);
  } catch (YAML::ParserException e) {
    CE_API_ERROR("Failed to load .catproj file '{0}'\n    {1}",
                 filePath.string(), e.what());
    return false;
  }

  auto projectNode = data["Project"];
  if (!projectNode)
    return false;

  config.Name = projectNode["Name"].as<std::string>();
  config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
  config.LastLoadedAssetPath = projectNode["LastLoadedAssetPath"].as<std::string>(config.AssetDirectory.generic_string());

  auto openScenes = projectNode["OpenScenes"];
  if (openScenes) {
    for (auto scene : openScenes) {
      AssetHandle sceneHandle = scene["AssetHandle"].as<UUID>();
      config.OpenScenes.push_back(sceneHandle);
    }
  }

  return true;
}

} // namespace CatEngine
