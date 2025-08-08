#include "cepch.h"
#include "VertexArray.h"

#include "RenderAPI.h"

#include "CatEngine/Platform/OpenGL/OpenGLVertexArray.h"

namespace CatEngine
{
    Ref<VertexArray> VertexArray::Create()
    {
        switch (RenderAPI::Get())
        {
            case RenderAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
            case RenderAPI::API::Vulkan: CE_API_ASSERT(false, "Vulkan is currently unsupported!"); return nullptr;
            default: CE_API_ASSERT(false, "CatEngine Headless is unsupported!");
        }
    }
}
