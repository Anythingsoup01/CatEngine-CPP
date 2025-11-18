#include "Input.h"

#include "CatScriptCore/InternalCalls/InternalCalls.h"

namespace CatRuntime
{
    bool Input::KeyPressed(KeyCode key)
    {
        return Input_IsKeyPressed(key);
    }

    bool Input::KeyReleased(KeyCode key)
    {
        return Input_IsKeyReleased(key);
    }
}
