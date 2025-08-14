#include "cepch.h"
#include "Buffer.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLBuffer.h"

namespace CatEngine
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }

    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
    {
        CE_PROFILE_FUNCTION();
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(indices, count);
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless not supported!"); return nullptr;
        }
    }
}
