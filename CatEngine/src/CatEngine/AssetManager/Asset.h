#pragma once

#include "CatEngine/Core/UUID.h"

#include "CatEngine/Types/TextureTypes.h"

namespace CatEngine
{
    using AssetHandle = UUID;

    enum class AssetType : uint16_t
    {
        None = 0,
        Scene,
        Texture2D,
    };

    static std::string AssetTypeToString(AssetType type)
    {
        switch (type) 
        {
            case AssetType::None:      return "None";
            case AssetType::Texture2D: return "Texture2D";
            case AssetType::Scene:     return "Scene"; 
        }
        return "<Invalid>";
    }

    static AssetType StringToAssetType(const std::string& type)
    {
        if (type == "None")      return AssetType::None;
        if (type == "Texture2D") return AssetType::Texture2D;
        if (type == "Scene")     return AssetType::Scene;

        return AssetType::None;
    }

    static std::string TextureParameterToString(const TextureParameter& param)
    {
        switch (param)
        {
            case TextureParameter::Linear: return "Linear";
            case TextureParameter::Nearest: return "Nearest";
            default: break;
        }

        CE_API_ASSERT(false, "Texture Parameter not Found!");
        return "NONE";
    }

    static std::string TextureWrapParameterToString(const TextureWrapParameter& param)
    {
        switch (param)
        {
            case TextureWrapParameter::Repeat: return "Repeat";
            case TextureWrapParameter::MirroredRepeat: return "MirroredRepeat";
            case TextureWrapParameter::ClampToBorders: return "ClampToBorders";
            case TextureWrapParameter::ClampToEdge: return "ClampToEdge";
            default: break;
        }

        CE_API_ASSERT(false, "Texture Wrap Parameter not Found!");
        return "NONE";
    }

    static std::string ImageFormatToString(const ImageFormat& format)
    {
        switch (format) 
        {
            case ImageFormat::R8: return "R8";
            case ImageFormat::RGB8: return "RGB8";
            case ImageFormat::RGBA8: return "RGBA8";
            default: break;
        }

        CE_API_ASSERT(false, "Image Format not Found!");
        return "NONE";
    }

    static TextureParameter StringToTextureParameter(const std::string& param)
    {
        if (param == "Linear") return TextureParameter::Linear;
        if (param == "Nearest") return TextureParameter::Nearest;


        CE_API_ASSERT(false, "Texture Parameter not Found!");
        return TextureParameter::NONE;
    }

    static TextureWrapParameter StringToTextureWrapParameter(const std::string& param)
    {
        if (param == "Repeat") return TextureWrapParameter::Repeat;
        if (param == "MirroredRepeat") return TextureWrapParameter::MirroredRepeat;
        if (param == "ClampToBorders") return TextureWrapParameter::ClampToBorders;
        if (param == "ClampToEdge") return TextureWrapParameter::ClampToEdge;


        CE_API_ASSERT(false, "Texture Parameter not Found!");
        return TextureWrapParameter::NONE;
    }

    static ImageFormat StringToImageFormat(const std::string& param)
    {
        if (param == "R8") return ImageFormat::R8;
        if (param == "RGB8") return ImageFormat::RGB8;
        if (param == "RGBA8") return ImageFormat::RGBA8;


        CE_API_ASSERT(false, "Texture Parameter not Found!");
        return ImageFormat::NONE;
    }


    class Asset
    {
    public:
        struct MetaData
        {
            std::string AssetName;
            AssetType Type = AssetType::None;
            std::filesystem::path FilePath;

            // TEXTURE DATA
            TextureParameter TextureMinFilter;
            TextureParameter TextureMagFilter;
            TextureWrapParameter TextureWrap;
            ImageFormat TextureFormat;

            operator bool() const { return Type != AssetType::None; }
        };

        AssetHandle m_Handle;
        virtual AssetType GetType() const = 0;
    };
}
