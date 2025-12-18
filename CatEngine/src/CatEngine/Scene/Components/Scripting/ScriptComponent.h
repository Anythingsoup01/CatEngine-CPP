#pragma once

#include <string>
#include "CatEngine/Core/UUID.h"


namespace CatEngine
{

	struct ScriptComponent
	{
        UUID ScriptID;
		std::string ClassName;
        int SelectedScript = 0;
        bool Loaded = false;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
        ~ScriptComponent() = default;

	};
}
