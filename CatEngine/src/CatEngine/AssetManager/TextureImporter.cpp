#include "CatEngine/Renderer/Texture.h"
#include "cepch.h"
#include "TextureImporter.h"

#include <stb_image.h>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{
    Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, Ref<Asset::MetaData>& metaData)
    {
        TextureSpecification spec;
        return ImportIconTexture(spec, metaData);
    }

    Ref<Texture2D> TextureImporter::ConstImportTexture2D(AssetHandle handle, const Ref<Asset::MetaData>& metaData)
    {
        TextureSpecification spec;
        Ref<Asset::MetaData> nonConstMetaData = metaData;
        return ImportIconTexture(spec, nonConstMetaData);
    }

    Ref<Texture2D> TextureImporter::ImportIconTexture(const TextureSpecification& spec, Ref<Asset::MetaData>& metaData)
    {
        CE_PROFILE_FUNCTION();
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        std::filesystem::path filePath = Project::GetAssetDirectory() / metaData->FilePath;
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

        TextureSpecification newSpec;
        newSpec.Width = width;
        newSpec.Height = height;
        newSpec.FilePath = filePath;
        newSpec.MinFilter = spec.MinFilter;
        newSpec.MagFilter = spec.MagFilter;
        newSpec.WrapOption = spec.WrapOption;

        if (spec.Format == ImageFormat::NONE)
        {
            if (channels == 3)
                newSpec.Format = ImageFormat::RGB8;
            else if (channels == 4)
                newSpec.Format = ImageFormat::RGBA8;
        }
        else
        {
            newSpec.Format = spec.Format;
        }

        std::filesystem::path fileName = filePath.filename();
        fileName.replace_extension("");

        Ref<Asset::TextureMetaData> textureMetaData = CreateRef<Asset::TextureMetaData>();
        textureMetaData->AssetName = fileName.generic_string();
        textureMetaData->FilePath = metaData->FilePath;
        textureMetaData->Type = metaData->Type;
        
        Ref<Texture2D> texture = Texture2D::Create(newSpec, data);
        
        data.Release();
        return texture;
    }
}
