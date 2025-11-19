#pragma once

namespace CatEngine
{
    enum class ImageFormat
	{
		R8,
		RGB8,
		RGBA8,
        NONE,
	};

    enum class TextureParameter
    {
        Linear,
        Nearest,
        NONE,
    };

    enum class TextureWrapParameter
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorders,
        NONE,
    };
}
