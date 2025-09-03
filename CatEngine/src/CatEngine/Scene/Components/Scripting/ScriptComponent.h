#pragma once
#include <string>

namespace CatEngine
{

	struct ScriptComponent
	{
		std::string ClassName;

        int SelectedScript = 0;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

	};
}
