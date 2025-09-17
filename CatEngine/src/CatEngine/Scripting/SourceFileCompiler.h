#pragma once

namespace CatEngine
{
    enum class KeyWordType
    {
        None = 0,
        Digit, String, Char, Bool, Vec2, Vec3, Vec4,
        TransformComponent, Rigidbody2DComponent
    };

    static std::vector<const char*> s_Keywords =
    {
        "float", "double",
		"char",
		"int16_t", "int32_t", "int", "int64_t", 
        "bool",
		"uint16_t", "uint32_t", "unsigned int", "uint64_t",
		"std::string",
		"glm::vec2", "glm::vec3", "glm::vec4",
		"TransformComponent", "Rigidbody2DComponent",
    };

    struct FileDescription
    {
        std::string Name;
        std::filesystem::path SharedObjectPath;
        std::filesystem::path SourceFilePath;
        std::filesystem::path ObjectPath;
    };

    class SourceFileCompiler
    {
    public:

        static void Init();

        static void AddFile(const std::filesystem::path& filePath);
        static void AddDirectory(const std::filesystem::path& directory);
        static void RemoveFile(const std::filesystem::path& filePath);

        static void CopyAndPrepareFile(const std::filesystem::path& filePath, bool initialLoad = false);
        static void CompileFile(FileDescription fd);

        static const std::vector<FileDescription>& GetCompiledFiles() { return m_CompiledFiles; }
    private:
        static std::string GetBuildCommandVariables(const FileDescription& fd);
    private:
        static inline std::vector<FileDescription> m_FilesToBeCompiled = {};
        static inline std::vector<FileDescription> m_CompiledFiles = {};

        static inline std::filesystem::path s_ProjectAssetPath;
    };

}
