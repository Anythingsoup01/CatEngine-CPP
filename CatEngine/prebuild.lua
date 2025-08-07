project "CatEngine"
    kind "StaticLib"
    language "C++"
    dialect "20"

    pch "src/cepch.h"

    files
    {
        "src/*.cpp",
        "src/*.h",
    }

    includedirs
    {
        "src",
        "$(WORKSPACEDIR)/vendor/spdlog/include",
        "$(WORKSPACEDIR)/vendor/glfw/include",
        "$(WORKSPACEDIR)/vendor/Glad/include",
        "$(WORKSPACEDIR)/vendor/imgui",
    }

    links
    {
        "ImGui",
        "glfw",
        "Glad",
        "GL",
    }

    defines
    {
        "GLFW_INCLUDE_NONE",
        "CE_DISPLAY_WAYLAND",
    }

    filter "configurations:Debug"
        defines "CE_DEBUG"

    filter "configurations:Release"
        defines "CE_RELEASE"
