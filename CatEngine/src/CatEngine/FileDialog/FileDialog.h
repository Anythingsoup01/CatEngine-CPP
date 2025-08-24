#pragma once

#include "nfd.h"

namespace CatEngine
{
    struct FilterElements
    {
        const char* Name;
        const char* Spec;
    };

    class FileDialog
    {
    public:
        static std::string OpenFile(const std::initializer_list<FilterElements>& filters);
        static std::string SaveFile(const std::initializer_list<FilterElements>& filters);
    };

}
