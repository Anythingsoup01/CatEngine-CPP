#pragma once

namespace CatEngine
{
    class SourceFileCompiler
    {
    public:
        static void Init();
        static void AddFile(const std::filesystem::path& filePath);
        static void AddDirectory(const std::filesystem::path& filePath);

    private:
        static std::string GetBuildCommandVariables(const std::filesystem::path& filePath);
        static void CompileFile(const std::filesystem::path& filePath);
    private:

        static inline std::filesystem::path s_ProjectAssetPath;
    };

}
