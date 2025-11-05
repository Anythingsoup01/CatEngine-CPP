Project = {
    name = "CatEditor",
    kind = "ConsoleApp",
    language = "C++",
    dialect = "20",

    files =
    {
        "src/*.cpp",
        "src/*.h",
    },

    includedirs =
    {
        "src",
        "${WORKSPACEDIR}CatEngine/src",
        "${WORKSPACEDIR}vendor/spdlog/include",
        "${WORKSPACEDIR}vendor/imgui",
        "${WORKSPACEDIR}vendor/glm",
        "${WORKSPACEDIR}vendor/entt",
        "${WORKSPACEDIR}vendor/Glad/include",
        "${WORKSPACEDIR}vendor/NFD-E/src/include",
        "${WORKSPACEDIR}vendor/yaml/include",
        "${WORKSPACEDIR}vendor/ImGuizmo",
        "${WORKSPACEDIR}vendor/Box2D/include",
        "${WORKSPACEDIR}vendor/Capybara/include",
    },

    links =
    {
        "CatEngine",
    },



    defines =
    {
        "GLFW_INCLUDE_NONE",
        "CE_DISPLAY_WAYLAND",
    },

    filters =
    {
        {
            name = "configurations:Debug",
            defines = "CE_DEBUG",
        },
        {
            name = "configurations:Release",
            defines = "CE_RELEASE",
        },
    },
}
