#pragma once

#include <string>


#include <glm/glm.hpp>

namespace CatEngine
{
	class ImGuiDraw
	{
	public:

		static bool ColorEdit4(const std::string& label, glm::vec4& values);
		static bool ColorEdit3(const std::string& label, glm::vec3& values);
		
		static bool Vec4(const std::string& label, glm::vec4& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f);
		static bool Vec3(const std::string& label, glm::vec3& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f);
		static bool Vec2(const std::string& label, glm::vec2& values, float step = 1.0f, float Min = 0.0f, float Max = 0.0f);
		static bool Vec1(const std::string& label, float& value, float step = 1.0f, float Min = 0.0f, float Max = 0.0f);

		static bool Int1(const std::string& label, int& value, int step = 1, int min = 0, int max = 0);

        static bool Combo(const std::string& label, int& selection, const std::vector<const char*>& list);
		static bool CheckBox(const std::string& label, bool& active);

        static void DataField(const std::string& label, const std::string& componentName);

        static void StaticString(const std::string& label, const std::string& str);
        static void StaticInt(const std::string& label, const int& i);

	};
}
