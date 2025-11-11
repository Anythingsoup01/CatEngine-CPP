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
    },

    flags = {
        "-fPIC",
        "-gdwarf-2",
    },
}
