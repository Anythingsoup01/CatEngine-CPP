#include "cepch.h"
#include "Formatter.h"

namespace CatEngine
{
    YAML::Emitter& operator<<(YAML::Emitter& out, const UUID& id)
	{
		out << YAML::Flow;
		out << (uint64_t)id;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
	YAML::Emitter& operator<<(YAML::Emitter& out, const entt::entity& id)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << (uint64_t)(uint32_t)id << YAML::EndSeq;
		return out;
	}
}
