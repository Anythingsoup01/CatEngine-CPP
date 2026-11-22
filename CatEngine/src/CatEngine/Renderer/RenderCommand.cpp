#include "cepch.h" // IWYU pragma: keep

#include "RenderCommand.h"

namespace CatEngine {

void RenderCommand::Init() {
  CE_PROFILE_FUNCTION();
  s_RenderAPI = RenderAPI::Create();
  s_RenderAPI->Init();
}

} // namespace CatEngine
