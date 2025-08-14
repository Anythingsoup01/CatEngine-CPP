#pragma once
#include <string>

namespace CatEngine
{

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

	};
}
