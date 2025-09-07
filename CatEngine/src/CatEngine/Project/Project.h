#pragma once

namespace CatEngine
{
    struct ProjectConfig
    {
        std::string Name = "Untitled";
        std::filesystem::path StartScene;
        std::filesystem::path AssetDirectory;
    };
    class Project
    {
    public:
        Project();

        static const std::filesystem::path& GetAssetDirectory()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project!");
            return s_ActiveProject->m_Config.AssetDirectory;
        }

        static ProjectConfig& GetConfig() 
        { 
            CE_API_ASSERT(s_ActiveProject, "No Active Project"); 
            return s_ActiveProject->m_Config; 
        }

        static const std::filesystem::path& GetProjectDirectory()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return s_ActiveProject->m_ActiveProjectDirectory;
        }

        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return GetProjectDirectory() / GetAssetDirectory() / path;
        }

        static std::filesystem::path GetAssetFileSystemPath()
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            return GetProjectDirectory() / GetAssetDirectory();
        }

        static void SetCurrentScene(const std::filesystem::path& path)
        {
            CE_API_ASSERT(s_ActiveProject, "No Active Project");
            s_ActiveProject->m_Config.StartScene = path;
        }

        static Ref<Project> New();
        static Ref<Project> Load(const std::filesystem::path& filePath); // TODO: Possibly make a bool
        static bool Save(const std::filesystem::path& filePath);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ActiveProjectDirectory;
        inline static Ref<Project> s_ActiveProject = CreateRef<Project>();
    };
}
