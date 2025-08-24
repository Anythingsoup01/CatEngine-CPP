#pragma once
#include <spdlog/fmt/bundled/format.h>
#include "spdlog/fmt/ostr.h"

namespace CatEngine::Math
{
	struct Vector2
	{
	public:
		float X, Y;
	};

	struct Vector3
	{
	public:
		float X, Y, Z;

		Vector3() = default;
		Vector3(float x, float y, float z)
			: X(x), Y(y), Z(z) {}
		Vector3(const Vector3& other) 
			: X(other.X), Y(other.Y), Z(other.Z) {}

		Vector3 operator+=(Vector3& other) const
		{
			return Vector3(X + other.X, Y + other.Y, Z + other.Z);
		}
		Vector3 operator-=(Vector3& other) const
		{
			return Vector3(X - other.X, Y - other.Y, Z - other.Z);
		}
	};

	
}


template<>
struct fmt::formatter<CatEngine::Math::Vector2>
{
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	template <typename FormatContext>
	auto format(const CatEngine::Math::Vector2& input, FormatContext& ctx) -> decltype(ctx.out()) {
		return format_to(ctx.out(),
			"({0}, {1})", input.X, input.Y);
	}
};

template<>
struct fmt::formatter<CatEngine::Math::Vector3>
{
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	template <typename FormatContext>
	auto format(const CatEngine::Math::Vector3& input, FormatContext& ctx) -> decltype(ctx.out()) {
		return format_to(ctx.out(),
			"({0}, {1}, {2})", input.X, input.Y, input.Z);
	}
};
