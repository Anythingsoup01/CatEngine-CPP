Project = {
	name = "CatScriptCore",
	kind = "SharedLib",
	language = "C++",
	dialect = "20",

	files = {
		"src/**.cpp",
	},

	includedirs = {
		"src",
	},

	flags = {
		"-fPIC",
		"-gdwarf-4",
		"-rdynamic",
	},
}
