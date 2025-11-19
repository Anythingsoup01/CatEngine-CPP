#include "ImGuiDraw.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

namespace CatEngine
{

    std::vector<std::string> names = {"name", "x", "y", "z", "a"};

	static float GetOffset(const size_t& size)
	{
		if (size >= 50)
		{
			return 150;
		}
		if (size >= 40)
		{
			return 120;
		}
		if (size >= 30)
		{
			return 90;
		}
		if (size >= 20)
		{
			return 60;
		}
		if (size >= 10)
		{
			return 30;
		}
		return 0;
	}

	template<typename T>
	static bool ImGuiColorEdit(T& values)
	{
		static_assert(false);
        return false;
	}

	template<>
	bool ImGuiColorEdit<glm::vec4>(glm::vec4& values)
	{
        bool triggered = false;
		ImGui::PushMultiItemsWidths(4, (ImGui::GetColumnWidth() * .95f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 3,0 });

		if (ImGui::ColorEdit4("##CO", glm::value_ptr(values), ImGuiColorEditFlags_NoInputs)) triggered = true;

		ImGui::PopStyleVar(1);
        return triggered;
	}

	template<>
	bool ImGuiColorEdit<glm::vec3>(glm::vec3& values)
	{
        bool triggered = false;
		ImGui::PushMultiItemsWidths(3, (ImGui::GetColumnWidth() * .95f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 3,0 });

		if (ImGui::ColorEdit3("##CO", glm::value_ptr(values), ImGuiColorEditFlags_NoInputs)) triggered = true;

		ImGui::PopStyleVar(1);
        return triggered;
	}

	template<typename T>
	bool ImGuiDragFloat(T& values, float step, float min, float max)
	{
		static_assert(false);
        return false;
	}

	template<>
    bool ImGuiDragFloat<glm::vec4>(glm::vec4& values, float step, float min, float max)
	{
        bool triggered = false;
		for (int i = 0; i < 4; i++)
		{
            ImGui::TableSetColumnIndex(i + 1);
			ImGui::PushID(i);
            ImGui::PushItemWidth(-1.0f);
            ImGui::Text("%s", names[i + 1].c_str());
            ImGui::SameLine();
			if (ImGui::DragFloat("##", &values[i], step, min, max, "%.2f")) triggered = true;
            ImGui::PopItemWidth();
			ImGui::PopID();
		}
        return triggered;
	}

	template<>
	bool ImGuiDragFloat<glm::vec3>(glm::vec3& values, float step, float min, float max)
	{
        bool triggered = false;
		for (int i = 0; i < 3; i++)
		{
            ImGui::TableSetColumnIndex(i + 1);
			ImGui::PushID(i);
            ImGui::PushItemWidth(-1.0f);
            ImGui::Text("%s", names[i + 1].c_str());
            ImGui::SameLine();
			if (ImGui::DragFloat("##", &values[i], step, min, max, "%.2f")) triggered = true;
            ImGui::PopItemWidth();
			ImGui::PopID();
		}
        return triggered;
	}

	template<>
	bool ImGuiDragFloat<glm::vec2>(glm::vec2& values, float step, float min, float max)
	{
        bool triggered = false;
		for (int i = 0; i < 2; i++)
		{
            ImGui::TableSetColumnIndex(i + 1);
			ImGui::PushID(i);
            ImGui::PushItemWidth(-1.0f);
            ImGui::Text("%s", names[i + 1].c_str());
            ImGui::SameLine();
			if (ImGui::DragFloat("##", &values[i], step, min, max, "%.2f")) triggered = true;
            ImGui::PopItemWidth();
			ImGui::PopID();
		}
        return triggered;
	}

	template<>
	bool ImGuiDragFloat<float>(float& value, float step, float min, float max)
	{
        bool triggered = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::PushID(0);
        ImGui::PushItemWidth(-1.0f);
        if (ImGui::DragFloat("##", &value, step, min, max, "%.2f")) triggered = true;
        ImGui::PopItemWidth();
        ImGui::PopID();
        return triggered;
	}

    template<typename T>
	bool ImGuiDragInt(T& values, int step, int min, int max)
	{
		static_assert(false);
        return false;
	}

    template<>
    bool ImGuiDragInt<int>(int& value, int step, int min, int max)
    {
        bool triggered = false;
        ImGui::TableSetColumnIndex(1);
        ImGui::PushItemWidth(-1.0f);
        ImGui::PushID(0);
        if (ImGui::DragInt("##", &value, step, min, max)) triggered = true;
        ImGui::PopItemWidth();
        ImGui::PopID();
        return triggered;
    }


	bool ImGuiDraw::ColorEdit4(const std::string& label, glm::vec4& values)
	{
		ImGui::PushID(label.c_str());
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, (ImGui::GetWindowWidth() / 4) + GetOffset(label.size()));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f);
		ImGui::Text("%s", label.c_str());
		ImGui::NextColumn();

		bool triggered = ImGuiColorEdit<glm::vec4>(values);

		ImGui::EndColumns();

		ImGui::PopID();
        return triggered;
	}

	bool ImGuiDraw::ColorEdit3(const std::string& label, glm::vec3& values)
	{
		ImGui::PushID(label.c_str());
		ImGui::Columns(1);
		ImGui::SetColumnWidth(0, (ImGui::GetWindowWidth() / 4) + GetOffset(label.size()));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.f);
		ImGui::Text("%s",label.c_str());
		ImGui::NextColumn();

		bool triggered = ImGuiColorEdit<glm::vec3>(values);

		ImGui::EndColumns();

		ImGui::PopID();
        return triggered;
	}

	bool ImGuiDraw::Vec4(const std::string& label, glm::vec4& values, float step, float min, float max)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());
		if (ImGui::BeginTable("##", 4))
        {

            for (int i = 0; i < 4; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
		    if (ImGuiDragFloat<glm::vec4>(values, step, min, max)) triggered = true;
            ImGui::EndTable();
        }
        

		ImGui::PopID();
        return triggered;
	}

	bool ImGuiDraw::Vec3(const std::string& label, glm::vec3& values, float step, float min, float max)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());

        if (ImGui::BeginTable("##", 4))
        {

            for (int i = 0; i < 4; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
		    if (ImGuiDragFloat<glm::vec3>(values, step, min, max)) triggered = true;
            ImGui::EndTable();
        }
        

		ImGui::PopID();

        return triggered;
	}

	bool ImGuiDraw::Vec2(const std::string& label, glm::vec2& values, float step, float min, float max)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());

        if (ImGui::BeginTable("##", 3))
        {

            for (int i = 0; i < 3; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
		    if (ImGuiDragFloat<glm::vec2>(values, step, min, max)) triggered = true;
            ImGui::EndTable();
        }
        

		ImGui::PopID();
        return triggered;
	}

	bool ImGuiDraw::Vec1(const std::string& label, float& value, float step, float min, float max)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());

        if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
		    if (ImGuiDragFloat<float>(value, step, min, max)) triggered = true;
            ImGui::EndTable();
        }
        

		ImGui::PopID();

        return triggered;

	}

	bool ImGuiDraw::Int1(const std::string& label, int& value, int step, int min, int max)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());

        if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
		    if (ImGuiDragInt<int>(value, step, min, max)) triggered = true;
            ImGui::EndTable();
        }
        

		ImGui::PopID();

        return triggered;

	}

    bool ImGuiDraw::Combo(const std::string& label, int& selection, const std::vector<const char*>& list)
    {
        bool triggered = false;
		ImGui::PushID(label.c_str());

        if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::PushItemWidth(-1.0f);
		    if (ImGui::Combo("##", &selection, list.data(), list.size())) triggered = true;
            ImGui::PopItemWidth();
            ImGui::EndTable();
        }
        

		ImGui::PopID();

        return triggered;

    }

	bool ImGuiDraw::CheckBox(const std::string& label, bool& active)
	{
        bool triggered = false;
		ImGui::PushID(label.c_str());

		if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Checkbox("##", &active)) triggered = true;
            ImGui::EndTable();
        }

		ImGui::PopID();
        return triggered;
	}

	void ImGuiDraw::Component(const std::string& label, const std::string& componentName)
	{
		ImGui::PushID(label.c_str());

		if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
            ImGui::TableSetColumnIndex(1);

            ImGui::PushItemWidth(-1.0f);
            ImGui::Button(componentName.c_str());
            ImGui::PopItemWidth();
            ImGui::EndTable();
        }
        

		ImGui::PopID();
	}

    void ImGuiDraw::StaticString(const std::string& label, const std::string& str)
    {
        ImGui::PushID(label.c_str());

		if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
            ImGui::TableSetColumnIndex(1);

            ImGui::PushItemWidth(-1.0f);
            ImGui::Text("%s", str.c_str());
            ImGui::PopItemWidth();
            ImGui::EndTable();
        }
        

		ImGui::PopID();

    }

    void ImGuiDraw::StaticInt(const std::string& label, const int& i)
    {
        ImGui::PushID(label.c_str());

		if (ImGui::BeginTable("##", 2))
        {

            for (int i = 0; i < 2; i++)
            {
                ImGui::TableSetupColumn(names[i].c_str(), ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s",label.c_str());
            ImGui::TableSetColumnIndex(1);

            ImGui::PushItemWidth(-1.0f);
            ImGui::Text("%i", i);
            ImGui::PopItemWidth();
            ImGui::EndTable();
        }
        

		ImGui::PopID();

    }
}
