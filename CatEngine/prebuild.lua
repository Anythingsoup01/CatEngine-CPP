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
        "$(WORKSPACEDIR)/vendor/stb/*.h",
        "$(WORKSPACEDIR)/vendor/stb/*.cpp",
        "$(WORKSPACEDIR)/vendor/entt/*.hpp",
        "$(WORKSPACEDIR)/vendor/ImGuizmo/*.cpp",
        "$(WORKSPACEDIR)/vendor/ImGuizmo/*.h",
    }

    includedirs
    {
        "src",
        "$(WORKSPACEDIR)/vendor/spdlog/include",
        "$(WORKSPACEDIR)/vendor/glfw/include",
        "$(WORKSPACEDIR)/vendor/Glad/include",
        "$(WORKSPACEDIR)/vendor/imgui",
        "$(WORKSPACEDIR)/vendor/glm",
        "$(WORKSPACEDIR)/vendor/stb",
        "$(WORKSPACEDIR)/vendor/entt",
        "$(WORKSPACEDIR)/vendor/NFD-E/src/include",
        "$(WORKSPACEDIR)/vendor/yaml/include",
        "$(WORKSPACEDIR)/vendor/ImGuizmo",
        "$(WORKSPACEDIR)/vendor/Box2D/include",
        "$(WORKSPACEDIR)/vendor/FileWatch",
    }

    links
    {
        "ImGui",
        "glfw",
        "Glad",
        "nfd",
        "yaml-cpp",
        "Box2D",
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
