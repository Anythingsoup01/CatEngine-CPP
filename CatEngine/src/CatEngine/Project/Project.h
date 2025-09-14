#pragma once

#include "CatEngine/AssetManager/AssetManagerBase.h"
#include "CatEngine/AssetManager/EditorAssetManager.h"
#include "CatEngine/AssetManager/RuntimeAssetManager.h"

namespace CatEngine
{
    struct ProjectConfig
    {
        std::string Name = "Untitled";
        AssetHandle StartScene;
        std::filesystem::path AssetDirectory;
        std::filesystem::path AssetRegistryPath; // Relative to the asset directory
    };

    class Project
    {
    public:
        Project();

        static const std::filesystem::path& GetProjectDirectory()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return s_ActiveProject->m_ActiveProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project!");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
        }

        static ProjectConfig& GetConfig() 
        { 
            CE_API_ASSERT(s_ActiveProject, "No Active Project"); 
            return s_ActiveProject->m_Config; 
        }

        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return GetProjectDirectory() / GetAssetDirectory() / path;
        }

        static std::filesystem::path GetAssetRegistryPath()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return GetAssetDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
        }

        static void SetCurrentScene(AssetHandle handle)
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            s_ActiveProject->m_Config.StartScene = handle;
        }

        static Ref<Project>& GetActive() { return s_ActiveProject; }

        Ref<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
        Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }
        Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }

        static Ref<Project> New();
        static Ref<Project> Load(const std::filesystem::path& filePath); // TODO: Possibly make a bool
        static bool Save(const std::filesystem::path& filePath);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ActiveProjectDirectory;
        inline static Ref<Project> s_ActiveProject = CreateRef<Project>();

        Ref<AssetManagerBase> m_AssetManager;
    };
}
