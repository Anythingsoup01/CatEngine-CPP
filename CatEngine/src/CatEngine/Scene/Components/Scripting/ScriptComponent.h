#pragma once
#include <string>

namespace CatEngine
{

	struct ScriptComponent
	{
		std::string ClassName;

        int SelectedScript = 0;

        bool Loaded = false;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

	};
}
