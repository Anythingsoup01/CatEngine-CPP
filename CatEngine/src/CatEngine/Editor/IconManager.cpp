#include <cepch.h>
#include "IconManager.h"

#include <stb_image.h>

#include "CatEngine/Core/Hash.h"

namespace CatEngine
{

    static Ref<Texture2D> ImportIconTexture(const std::filesystem::path& filePath)
    {
        CE_PROFILE_FUNCTION();
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        Buffer data;
        {
            CE_PROFILE_SCOPE("stbi_load - ImportIconTexture");
            data.Data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        }
        if (!data)
        {
            CE_API_ASSERT(false, "Failed to load texture from filepath {}", filePath.string());
            return nullptr;
        }

        // TODO: Rework this when HDR gets supported!
        data.Size = width * height * channels;

        TextureSpecification newSpec;
        newSpec.Width = width;
        newSpec.Height = height;
        newSpec.FilePath = filePath;
        newSpec.MinFilter = TextureParameter::Linear;
        newSpec.MagFilter = TextureParameter::Linear;
        newSpec.WrapOption = TextureWrapParameter::Repeat;

        if (channels == 3)
            newSpec.Format = ImageFormat::RGB8;
        else if (channels == 4)
            newSpec.Format = ImageFormat::RGBA8;

        Ref<Texture2D> texture = Texture2D::Create(newSpec, data);
        
        data.Release();
        return texture;
    }

    void IconManager::Init()
    {
        AddIcon("Resources/Icons/Editor/Start-Runtime.png");
        AddIcon("Resources/Icons/Editor/Pause-Runtime.png");
		AddIcon("Resources/Icons/Editor/Pause-Runtime-Selected.png");
		AddIcon("Resources/Icons/Editor/NextFrame-Runtime.png");
		AddIcon("Resources/Icons/Editor/Stop-Runtime.png");
		AddIcon("Resources/Icons/Editor/Start-Simulation.png");

        AddIcon("Resources/Icons/DirectoryIcon.png");
		AddIcon("Resources/Icons/ScriptFileIcon.png");
    }

    void IconManager::Shutdown()
    {
        s_IconMap.clear();
    }

    Ref<Texture2D> IconManager::AddIcon(const std::filesystem::path& filePath, const std::string& nameOverload)
    {
        std::filesystem::path fileName = filePath.filename();
        fileName.replace_extension("");

        std::string_view name = nameOverload.empty() ? fileName.c_str() : nameOverload.c_str();
        uint32_t nameHash = Hash::GenerateFNVHash(name);

        if (s_IconMap.contains(nameHash))
        {
            CE_API_ERROR("Icon '{}' already exists! If this is a custom icon use nameOverload to specify!", name.data());
            return nullptr;
        }

        Ref<Texture2D> out = ImportIconTexture(filePath);
        if (!out)
            return nullptr;

        s_IconMap[nameHash] = out;

        return out;
    }

    Ref<Texture2D> IconManager::GetIcon(const char* iconName)
    {
        uint32_t nameHash = Hash::GenerateFNVHash(iconName);

        if (s_IconMap.contains(nameHash))
            return s_IconMap[nameHash];

        CE_API_ERROR("Icon '{}' doesn't exist!", iconName);
        return nullptr;
    }

} 
