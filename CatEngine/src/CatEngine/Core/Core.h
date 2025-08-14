#pragma once

#include <memory>

#ifdef CE_PLATFORM_WINDOWS
#   ifdef CE_DEBUG
#       define CE_PROFILE
#       define CE_API_ASSERT(x, ...) if (!(x)) { CatEngine::Log::GetAPILogger()->critical(__VA_ARGS__); __debugbreak(); }
#       define CE_ASSERT(x) if (!(x)) __debugbreak();
#   endif
#elif defined(CE_PLATFORM_LINUX)
#   ifdef CE_DEBUG
#       define CE_PROFILE
#       include <signal.h>
#       define CE_API_ASSERT(x, ...) if (!(x)) { CatEngine::Log::GetAPILogger()->critical(__VA_ARGS__); raise(SIGTRAP); }
#       define CE_ASSERT(x) if(!(x)) raise(SIGTRAP);
#   endif
#endif

#define BIT(x) 1 << x

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

typedef uint32_t RendererID;

#define CE_API_ASSERT(x, ...) if (!(x)) { CatEngine::Log::GetAPILogger()->critical(__VA_ARGS__); raise(SIGTRAP); }
namespace CatEngine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
