#pragma once

#include <string>

#include "CatEngine/Core/UUID.h"

namespace CatEngine
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
        IDComponent(const UUID& id)
            : ID(id) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct LayerComponent
	{
		std::string Layer;

		LayerComponent() = default;
		LayerComponent(const LayerComponent&) = default;
		LayerComponent(const std::string& layer)
			: Layer(layer) {}

	};

	struct NameComponent
	{
		std::string Name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& name)
			: Name(name) {}
	};

	//struct ObjectStateComponent
	//{
	//	bool IsActive = true;

	//	ObjectStateComponent() = default;
	//	ObjectStateComponent(const ObjectStateComponent&) = default;
	//	ObjectStateComponent(bool& isActive)
	//		: IsActive(isActive) {}
	//};
}
