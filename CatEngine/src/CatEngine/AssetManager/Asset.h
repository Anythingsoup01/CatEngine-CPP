#pragma once

#include "CatEngine/Core/UUID.h"

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

    class Asset
    {
    public:
        struct MetaData
        {
            AssetType Type = AssetType::None;
            std::filesystem::path FilePath;

            operator bool() const { return Type != AssetType::None; }
        };

        AssetHandle m_Handle;

        virtual AssetType GetType() const = 0;
    };
}
