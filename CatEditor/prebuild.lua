project "CatEditor"
    kind "ConsoleApp"
    language "C++"
    dialect "20"

    files
    {
        "src/*.cpp",
        "src/*.h",
    }

    includedirs
    {
        "src",
        "$(WORKSPACEDIR)/CatEngine/src",
        "$(WORKSPACEDIR)/vendor/spdlog/include",
        "$(WORKSPACEDIR)/vendor/imgui",
        "$(WORKSPACEDIR)/vendor/glm",
        "$(WORKSPACEDIR)/vendor/entt",
        "$(WORKSPACEDIR)/vendor/Glad/include",
        "$(WORKSPACEDIR)/vendor/NFD-E/src/include",
        "$(WORKSPACEDIR)/vendor/yaml/include",
        "$(WORKSPACEDIR)/vendor/ImGuizmo",
    }

    links
    {
        "CatEngine",
    }

    defines
    {
        "GLFW_INCLUDE_NONE",
        "CE_DISPLAY_WAYLAND",
    }

    filter "configurations:Debug"
        define "CE_DEBUG"

    filter "configurations:Release"
        defines "CE_RELEASE"
