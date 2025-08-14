#include "cepch.h"
#include "CatEngine/Renderer/Framebuffer.h"

#include "CatEngine/Renderer/RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLFramebuffer.h"

namespace CatEngine {

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		CE_PROFILE_FUNCTION();

		switch (RenderAPI::Get())
		{
            case RenderAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
            case RenderAPI::API::Vulkan:  CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!") return nullptr;
		}

		CE_API_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
