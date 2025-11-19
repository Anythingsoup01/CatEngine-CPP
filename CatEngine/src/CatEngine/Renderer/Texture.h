#pragma once

#include "CatEngine/AssetManager/Asset.h"

#include "CatEngine/Core/Buffer.h"

#include "CatEngine/Types/TextureTypes.h"

namespace CatEngine
{


	struct TextureSpecification
	{
        std::filesystem::path FilePath;
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::NONE;
        TextureParameter MinFilter = TextureParameter::Linear;
        TextureParameter MagFilter = TextureParameter::Linear;
        TextureWrap WrapOption = TextureWrap::Repeat;
		bool GenerateMips = true;
	};

	class Texture : public Asset
	{
	public:

		virtual ~Texture() = default;

        virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        virtual void SetData(Buffer data) = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;

        virtual bool IsLoaded() const = 0;


        virtual bool operator==(const Texture& other) const = 0;

    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const TextureSpecification& specification = TextureSpecification(), Buffer data = Buffer());

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        virtual AssetType GetType() const { return GetStaticType(); }

        virtual void RecreateTextureWithNewSpecification(const TextureSpecification& spec, Buffer data) = 0;
    };

    class TextureHelper
    {
    public:
        static Buffer BufferTexture(const std::filesystem::path& filePath, int& width, int& height);
    };
}
