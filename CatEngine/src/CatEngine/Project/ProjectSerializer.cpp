#include "cepch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace CatEngine
{
    bool ProjectSerializer::Serialize(const std::filesystem::path& filePath)
    {
		CE_PROFILE_FUNCTION();

        const auto& config = Project::GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value;
        out << YAML::BeginMap; // Project
        out << YAML::Key << "Name" << YAML::Value << config.Name;
        out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
        out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
        out << YAML::EndMap;

		std::ofstream fout(filePath);
		fout << out.c_str();

        return true;
    }
 
    bool ProjectSerializer::Deserialize(const std::filesystem::path& filePath)
    {
        auto& config = Project::GetConfig();

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filePath);
        }
        catch (YAML::ParserException e)
        {
            CE_API_ERROR("Failed to load .catproj file '{0}'\n    {1}", filePath.string(), e.what());
            return false;
        }

        auto projectNode = data["Project"];
        if (!projectNode)
            return false;

        config.Name = projectNode["Name"].as<std::string>();
        config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
        config.StartScene = projectNode["StartScene"].as<std::string>();

        CE_API_INFO("{}", config.AssetDirectory.string());
        return true;
    }
}
