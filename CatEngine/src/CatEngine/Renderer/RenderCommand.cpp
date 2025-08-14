#include "cepch.h"
#include "RenderCommand.h"

namespace CatEngine
{
    void RenderCommand::Init()
    {
        CE_PROFILE_FUNCTION();
        s_RenderAPI = RenderAPI::Create();
        s_RenderAPI->Init();
    }
}
