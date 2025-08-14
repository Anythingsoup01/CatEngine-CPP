#include "cepch.h"
#include "OpenGLRenderAPI.h"

#include <glad/glad.h>

namespace CatEngine
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         CE_API_CRITICAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       CE_API_ERROR(message); return;
		case GL_DEBUG_SEVERITY_LOW:          CE_API_WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: CE_API_TRACE(message); return;
		}

		CE_API_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRenderAPI::Init()
	{
        CE_PROFILE_FUNCTION();

	#ifdef CE_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);


	}

    void OpenGLRenderAPI::SetViewport(uint32_t width, uint32_t height)
    {
        CE_PROFILE_FUNCTION();
        glViewport(0, 0, width, height);
    }

    void OpenGLRenderAPI::Clear(const glm::vec4& clearColor)
    {
        CE_PROFILE_FUNCTION();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void OpenGLRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
    {
        CE_PROFILE_FUNCTION();
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRenderAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, const uint32_t& indexCount)
    {
        CE_PROFILE_FUNCTION();
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }
}
