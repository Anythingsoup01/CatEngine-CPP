#include "cepch.h"
#include "ImGuiLayer.h"

#include "CatEngine/Core/Core.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include <GLFW/glfw3.h>

#include "CatEngine/Core/Application.h"

#include <imgui_internal.h>

namespace CatEngine {
	

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer") {}

	void ImGuiLayer::OnAttach()
	{
		// Setting up ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

#       ifndef CE_DISPLAY_WAYLAND
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#       endif

		ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;

		// ImGui Style
		ImGui::StyleColorsDark();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();
		SetDefaultEditorStyle();

		Application& app = Application::Get();
		GLFWwindow* window = app.GetWindow().Get();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");
	}

	void ImGuiLayer::OnUpdate(Time ts)
	{
		ImGui::GetCurrentContext()->NavWindowingToggleLayer = false;
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockKeyboardEvents) {
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(KeyboardEventCategory) & io.WantCaptureKeyboard;
		}
		if (m_BlockMouseEvents) {
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(MouseEventCategory) & io.WantCaptureMouse;
		}
	}

	
	void ImGuiLayer::Begin()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
	}
	
	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
	void ImGuiLayer::SetDarkThemeColors()
	{

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.13f, 0.13f, 0.13f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.143f, 0.143f, 0.143f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.173f, 0.173f, 0.173f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.155f, 0.155f, 0.155f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.175f, 0.175f, 0.175f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.157f, 0.157f, 0.157f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.3f, 0.3f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.257f, 0.257f, 0.257f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.235f, 0.235f, 0.235f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.235f, 0.235f, 0.235f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.335f, 0.335f, 0.335f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.235f, 0.235f, 0.235f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.235f, 0.235f, 0.235f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.175f, 0.175f, 0.175f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.175f, 0.175f, 0.175f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.175f, 0.175f, 0.175f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.176f, 0.176f, 0.176f, 1.0f };
	}
	void ImGuiLayer::SetDefaultEditorStyle()
	{
		auto& style = ImGui::GetStyle();

		style.WindowPadding = { 5, 5 };
		style.WindowBorderSize = 0;
		style.WindowRounding = 2;

		style.ChildBorderSize = 0;
		style.ChildRounding = 2;

		style.PopupBorderSize = 0;
		style.PopupRounding = 1;

		style.FramePadding = { 1, 2 };
		style.FrameBorderSize = 0;
		style.FrameRounding = 2;

		style.TabBorderSize = 0;
		style.TabBarBorderSize = 1;
		style.TabRounding = 6;

		style.ScrollbarSize = 10;
		style.ScrollbarRounding = 0;

		style.GrabMinSize = 5;
		style.GrabRounding = 0;

		style.WindowTitleAlign = { 0.5, 0.75f };
		style.WindowMenuButtonPosition = ImGuiDir_Right;

		style.ButtonTextAlign = { 0.5f, 0.5f };

		style.SeparatorTextBorderSize = 1;
		style.SeparatorTextAlign = { 0, 0 };
		style.SeparatorTextPadding = { 20, 0 };

		style.DockingSeparatorSize = 2;

		style.ItemSpacing = { 5, 5 };
		style.ItemInnerSpacing = { 5, 0 };
		style.TouchExtraPadding = { 0, 0 };
		style.IndentSpacing = 10;
	}
}
