#pragma once

namespace CatEngine
{

    struct FileDescription
    {
        std::string Name;
        std::string FileName;
        std::filesystem::path Path;
        std::filesystem::path RelativePath;
        std::filesystem::path ObjPath;
        std::filesystem::path CompilePath;
    };

    class SourceFileCompiler
    {
    public:

        static void Init();

        static void AddFile(const std::filesystem::path& filePath);
        static void AddDirectory(const std::filesystem::path& directory);
        static void RemoveFile(const std::filesystem::path& filePath);
        static std::unordered_map<std::filesystem::path, FileDescription> GetIntermediateFiles();
        static void CopyAndPrepareFiles();
        static void CompileFiles();
    private:
        static std::string GetBuildCommandVariables(const FileDescription& fd);
    private:
        static inline std::vector<std::filesystem::path> m_FilesToBePrepared = {};
        static inline std::unordered_map<std::filesystem::path,FileDescription> m_FilesToBeCompiled = {};
        static inline std::unordered_map<std::filesystem::path,FileDescription> m_CompiledFiles = {};

        static inline bool m_Preparing = false;
        static inline bool m_NeedsRecompiled = true;
    };

}
