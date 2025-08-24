#pragma once

#include <string>


#include <glm/glm.hpp>

namespace CatEngine
{
	class ImGuiDraw
	{
	public:

		static void DrawColorEdit4Control(const std::string& label, glm::vec4& values, float columnWidth = 100.f);
		static void DrawColorEdit3Control(const std::string& label, glm::vec3& values, float columnWidth = 100.f);
		
		
		static void DrawVec4Control(const std::string& label, glm::vec4& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f, float columnWidth = 100.f);
		static void DrawVec3Control(const std::string& label, glm::vec3& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f, float columnWidth = 100.f);
		static void DrawVec2Control(const std::string& label, glm::vec2& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f, float columnWidth = 100.f);
		static void DrawVec1Control(const std::string& label, float& value, float step = 1.0f, float Min = 0.0f, float Max = 0.0f, float columnWidth = 100.f);
		
		static void DrawCheckBox(const std::string& label, bool& active, float columnWidth = 100.f);
	};
}
