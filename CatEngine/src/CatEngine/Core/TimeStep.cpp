#include "cepch.h" // IWYU pragma: keep
#include "TimeStep.h"

#include <GLFW/glfw3.h>

namespace CatEngine {
float Time::GetTime() { return (float)glfwGetTime(); }
} // namespace CatEngine
