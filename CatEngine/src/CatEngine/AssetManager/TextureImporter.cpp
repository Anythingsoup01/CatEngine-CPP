#include "cepch.h"
#include "TextureImporter.h"

#include <stb_image.h>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{
    Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const Asset::MetaData& metaData)
    {

        return ImportIconTexture(Project::GetAssetDirectory() / metaData.FilePath);
    }
    Ref<Texture2D> TextureImporter::ImportIconTexture(const std::filesystem::path& filePath)
    {
        CE_PROFILE_FUNCTION();
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        Buffer data;
        {
            CE_PROFILE_SCOPE("stbi_load - TextureImporter::ImportTexture2D");
            data.Data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        }
        if (!data)
        {
            CE_API_ASSERT(false, "Failed to load texture from filepath {}", filePath.string());
            return nullptr;
        }

        // TODO: Rework this when HDR gets supported!
        data.Size = width * height * channels;

        TextureSpecification spec;
        spec.Width = width;
        spec.Height = height;

        switch (channels) 
        {
            case 3:
                spec.Format = ImageFormat::RGB8;
                break;
            case 4:
                spec.Format = ImageFormat::RGBA8;
                break;
        }
        
        Ref<Texture2D> texture = Texture2D::Create(spec, data);
        
        data.Release();
        return texture;
    }
}
