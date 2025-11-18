#pragma once

#include "KeyCodes.h"

namespace CatRuntime
{
    class Input 
    {
    public:
        // This is a continuos check to see if the user is pressing a key
        static bool KeyPressed(KeyCode key);

        // This is a One time check for when the key is up
        static bool KeyReleased(KeyCode key);
    };
}
