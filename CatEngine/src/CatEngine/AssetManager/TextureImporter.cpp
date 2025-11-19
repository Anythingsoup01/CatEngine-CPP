#include "cepch.h"
#include "TextureImporter.h"

#include <stb_image.h>

#include "CatEngine/Project/Project.h"

namespace CatEngine
{

    static int ImageFormatToChannels(const ImageFormat& format)
    {
        switch (format)
        {
            case ImageFormat::R8: return 1;
            case ImageFormat::RGB8: return 3;
            case ImageFormat::RGBA8: return 4;
            default: break;
        }

        CE_API_ASSERT(false, "ImageFormat not supported!");
        return 0;
    }

    Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, Asset::MetaData& metaData)
    {
        TextureSpecification spec;
        return ImportIconTexture(spec, metaData);
    }

    Ref<Texture2D> TextureImporter::ConstImportTexture2D(AssetHandle handle, const Asset::MetaData& metaData)
    {
        TextureSpecification spec;

        spec.MinFilter = metaData.TextureMinFilter;
        spec.MagFilter = metaData.TextureMagFilter;
        spec.WrapOption = metaData.TextureWrap;
        spec.Format = metaData.TextureFormat;

        Asset::MetaData nonConstMetaData = metaData;

        return ImportIconTexture(spec, nonConstMetaData);
    }

    Ref<Texture2D> TextureImporter::ImportIconTexture(const TextureSpecification& spec, Asset::MetaData& metaData)
    {
        CE_PROFILE_FUNCTION();
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        std::filesystem::path filePath = Project::GetAssetDirectory() / metaData.FilePath;
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
        newSpec.MinFilter = spec.MinFilter == TextureParameter::NONE ? TextureParameter::Linear : spec.MinFilter;
        newSpec.MagFilter = spec.MagFilter == TextureParameter::NONE ? TextureParameter::Linear : spec.MagFilter;
        newSpec.WrapOption = spec.WrapOption == TextureWrapParameter::NONE ? TextureWrapParameter::Repeat : spec.WrapOption;


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

        if (spec.Format == ImageFormat::NONE)
        {
            std::filesystem::path fileName = filePath.filename();
            fileName.replace_extension("");

            metaData.AssetName = fileName.generic_string();
            metaData.TextureMinFilter = newSpec.MinFilter;
            metaData.TextureMagFilter = newSpec.MagFilter;
            metaData.TextureWrap = newSpec.WrapOption;
            metaData.TextureFormat = newSpec.Format;
        }
        Ref<Texture2D> texture = Texture2D::Create(newSpec, data);
        
        data.Release();
        return texture;
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

}
