Project = {
    name = "CatScriptCore",
    kind = "SharedLib",
    language = "C++",
    dialect = "20",

    files = {
        "src/*.cpp",
        "src/*.h",
    },

    includedirs = {
        "src",
        "${WORKSPACEDIR}CatEngine/src",
        "${WORKSPACEDIR}vendor/spdlog/include",
        "${WORKSPACEDIR}vendor/glm",
        "${WORKSPACEDIR}vendor/entt",
        "${WORKSPACEDIR}vendor/Box2D/include",
    },

    flags = {
        "-fPIC",
        "-gdwarf-2",
    },

    links = {
        "CatEngine"
    }
}
