Project = {
	name = "CatEngine",
	kind = "StaticLib",
	language = "C++",
	dialect = "20",

	pch = "src/cepch.h",

	files = {
		"src/**.cpp",
		"../vendor/glm/glm/**.hpp",
		"../vendor/glm/glm/**.cpp",
		"../vendor/stb/**.h",
		"../vendor/stb/**.cpp",
		"../vendor/entt/**.hpp",
		"../vendor/ImGuizmo/**.cpp",
		"../vendor/ImGuizmo/**.h",
	},

	includedirs = {
		"src",
		"../vendor/spdlog/include",
		"../vendor/glfw/include",
		"../vendor/Glad/include",
		"../vendor/imgui",
		"../vendor/glm",
		"../vendor/stb",
		"../vendor/entt",
		"../vendor/NFD-E/src/include",
		"../vendor/yaml/include",
		"../vendor/ImGuizmo",
		"../vendor/Box2D/include",
		"../vendor/FileWatch",
		"../vendor/Capybara/Capybara/src",
	},

	links = {
		"ImGui",
		"glfw",
		"Glad",
		"nfd",
		"yaml-cpp",
		"Box2D",
		"GL",
		"Capybara",
	},

	defines = {
		"GLFW_INCLUDE_NONE",
		"CE_DISPLAY_WAYLAND",
	},

	filters = {
		{
			type = "configurations",
      param = "Debug",
			defines = "CE_DEBUG",
		},
		{
			type = "configurations",
      param = "Release",
			defines = "CE_RELEASE",
		},
		{
      type = "system",
      param = "linux",
			links = "X11",
		},
	},
}
