#include "cepch.h"
#include "UniformBuffer.h"
#include "RenderAPI.h"
#include "CatEngine/Platform/OpenGL/OpenGLUniformBuffer.h"

namespace CatEngine
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		CE_PROFILE_FUNCTION();

		switch (RenderAPI::Get())
		{
		    case RenderAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
		    case RenderAPI::API::Vulkan:  CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless is not supported!"); return nullptr;
        }
	}
}
