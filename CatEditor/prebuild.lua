Project = {
	name = "CatEditor",
	kind = "ConsoleApp",
	language = "C++",
	dialect = "20",

	files = {
		"src/**.cpp",
	},

	includedirs = {
		"src",
		"../CatEngine/src",
		"../vendor/spdlog/include",
		"../vendor/imgui",
		"../vendor/glm",
		"../vendor/entt",
		"../vendor/Glad/include",
		"../vendor/NFD-E/src/include",
		"../vendor/yaml/include",
		"../vendor/ImGuizmo",
		"../vendor/Box2D/include",
		"../vendor/Capybara/Capybara/src",
	},

	links = {
		"CatEngine",
	},

	defines = {
		"GLFW_INCLUDE_NONE",
		"CE_DISPLAY_WAYLAND",
	},

	filters = {
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
