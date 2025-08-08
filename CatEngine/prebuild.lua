project "CatEngine"
    kind "StaticLib"
    language "C++"
    dialect "20"

    pch "src/cepch.h"

    files
    {
        "src/*.cpp",
        "src/*.h",
        "$(WORKSPACEDIR)/vendor/glm/glm/*.hpp",
        "$(WORKSPACEDIR)/vendor/glm/glm/*.cpp",
    }

    includedirs
    {
        "src",
        "$(WORKSPACEDIR)/vendor/spdlog/include",
        "$(WORKSPACEDIR)/vendor/glfw/include",
        "$(WORKSPACEDIR)/vendor/Glad/include",
        "$(WORKSPACEDIR)/vendor/imgui",
        "$(WORKSPACEDIR)/vendor/glm",
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
