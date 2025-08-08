#include "cepch.h"
#include "RenderCommand.h"

namespace CatEngine
{
    void RenderCommand::Init()
    {
        s_RenderAPI = RenderAPI::Create();
    }
}
