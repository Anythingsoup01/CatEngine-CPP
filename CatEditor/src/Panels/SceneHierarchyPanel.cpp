#include "SceneHierarchyPanel.h"
#include "CatEngine/Scene/Components/Components.h"
#include "ImGui/ImGuiDraw.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <filesystem>

namespace CatEngine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetContext(scene);
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& scene)
	{
		m_SelectionContext = {};
		m_Context = scene;

	}
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Hierarchy");
		for (auto entityID : m_Context->GetReg().view<entt::entity>())
		{
			Entity entity{ entityID , m_Context.get() };
			DrawEntityNode(entity);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectionContext = {};
		}

		if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Create Empty GameObject"))
				{
					const auto& entity = m_Context->CreateEntity("GameObject");
					m_SelectionContext = entity;
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Create Camera"))
				{
					const auto& entity = m_Context->CreateEntity("Camera");
					entity.AddComponent<CameraComponent>().Primary = false;
					m_SelectionContext = entity;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
		ImGui::Begin("Inspector");
		if (m_SelectionContext)
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			DrawComponents(m_SelectionContext);

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (((float)contentRegionAvailable.x - ((float)contentRegionAvailable.x / 2.f)) * .5f));

			bool Components2DAvaliable = ((!m_SelectionContext.HasComponent<SpriteRendererComponent>()));

			if (ImGui::Button("Add Component", ImVec2{ contentRegionAvailable.x / 2.f, lineHeight }))
				ImGui::OpenPopup("AddComponent");
			
			if (ImGui::BeginPopup("AddComponent"))
			{
				DisplayAddComponentEntry<CameraComponent>("Camera Component");
				if (Components2DAvaliable)
				{
					bool opened = ImGui::TreeNodeEx("2D Components", treeNodeFlags);

					if (opened)
					{
						DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer Component");
						ImGui::TreePop();
					}
				}
				DisplayAddScriptComponent("Script");
				ImGui::EndPopup();
			}

		}
		ImGui::End();
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& name = entity.GetComponent<NameComponent>().Name;

		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
				entityDeleted = true;

			ImGui::EndPopup();
		}
		
		if (opened)
		{
			ImGui::TreePop();
		}
		if (entityDeleted)
		{
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
			m_Context->DeleteEntity(entity);
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canRemove = true)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();


			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 10, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight);
			if (ImGui::Button(":", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("Component Settings");
			}
			bool removeComponent = false;
			if (ImGui::BeginPopup("Component Settings"))
			{
				if (canRemove)
				{
					if (ImGui::MenuItem("Remove Component"))
					{
						removeComponent = true;
						ImGui::CloseCurrentPopup();
					}
				}
				if (ImGui::MenuItem("Reset Component"))
				{
					component = T();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}
			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity selection)
	{
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 10,1 });
			ImGui::Columns(2);

			auto& tag = selection.GetComponent<TagComponent>().Tag;

			char tagBuffer[256];
			memset(tagBuffer, 0, sizeof(tagBuffer));
			strncpy(tagBuffer, tag.c_str(), 256);
			ImGui::Text("Tag");
			ImGui::SameLine();
			if (ImGui::InputText("##T", tagBuffer, sizeof(tagBuffer)))
			{
				tag = std::string(tagBuffer);
			}

			ImGui::NextColumn();

			auto& layer = selection.GetComponent<LayerComponent>().Layer;

			char layerBuffer[256];
			memset(layerBuffer, 0, sizeof(layerBuffer));
			strncpy(layerBuffer, layer.c_str(), 256);
			ImGui::Text("Layer");
			ImGui::SameLine();
			if (ImGui::InputText("##L", layerBuffer, sizeof(layerBuffer)))
			{
				layer = std::string(layerBuffer);
			}

			ImGui::Columns(1);

			auto& name = selection.GetComponent<NameComponent>().Name;

			char nameBuffer[256];
			memset(nameBuffer, 0, sizeof(nameBuffer));
			strncpy(nameBuffer, name.c_str(), 256);
			ImGui::Text("Name");
			ImGui::SameLine();
			if (ImGui::InputText("##N", nameBuffer, sizeof(nameBuffer)))
			{
				name = std::string(nameBuffer);
			}
			ImGui::PopStyleVar();
		}

		DrawComponent<TransformComponent>("Transform", selection, [](auto& component) {
			ImGuiDraw::DrawVec3Control("Position", component.Position, 0.1f, 0.0f, 0.0f);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			ImGuiDraw::DrawVec3Control("Rotation", rotation, 0.1f, 0.0f, 0.0f);
			component.Rotation = glm::radians(rotation);
			ImGuiDraw::DrawVec3Control("Scale", component.Scale, 0.1f, 0.0f, 0.0f);
		}, false);
		 
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", selection, [](auto& component) 
		{
			ImGuiDraw::DrawColorEdit4Control("Color", component.Color);
			
			ImGui::Button("Texture", ImVec2{100.f, 100.f});
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path("assets") / path;
					Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded())
						component.Texture = texture;
					else
						CE_API_WARN("Could not load texture {0}", texturePath.filename().string());
				}
				ImGui::EndDragDropTarget();
			}

			ImGuiDraw::DrawVec1Control("Tiling Factor", component.TilingFactor, 0.1f, 0.f, 100.f);
		});

        DrawComponent<CameraComponent>("Camera", selection, [](auto& component) 
		{
			auto& camera = component.Camera;

			const char* projectionTypeString[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeString[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeString[i];
					if (ImGui::Selectable(projectionTypeString[i], &isSelected))
					{
						currentProjectionTypeString = projectionTypeString[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();

				}
				ImGui::EndCombo();
			}
			auto& primary = component.Primary;
			ImGui::Checkbox("Primary Camera", &primary);

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspectice)
			{
				auto vertFov = glm::degrees(camera.GetPerspectiveVerticalFov());
				if (ImGui::DragFloat("Vertical FOV", &vertFov))
					camera.SetPerspectiveVerticalFov(glm::radians(vertFov));

				auto nearClip = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip, .01f, 0.f))
					camera.SetPerspectiveNearClip(nearClip);

				auto farClip = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip, .01f, 0.f))
					camera.SetPerspectiveFarClip(farClip);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				auto size = camera.GetOrthographicSize();
				if (ImGui::DragFloat("FOV", &size))
					camera.SetOrthographicSize(size);

				auto nearClip = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Clip", &nearClip, .01f, 0.f))
					camera.SetOrthographicNearClip(nearClip);

				auto farClip = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Clip", &farClip, .01f, 0.f))
					camera.SetOrthographicFarClip(farClip);
			}

            auto& fixed = component.FixedAspectRatio;
            ImGui::Checkbox("Fixed Aspect Ratio", &fixed);
            
		});



		DrawComponent<ScriptComponent>("Script", selection, [selection, this](auto& component) mutable
		{

		});
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
	template<typename T, typename O>
	void SceneHierarchyPanel::DisplayAddComponentEntries(const std::string& entryOneName, const std::string& entryTwoName)
	{
		if (!m_SelectionContext.HasComponent<T>() && !m_SelectionContext.HasComponent<O>())
		{
			if (ImGui::MenuItem(entryOneName.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
		if (!m_SelectionContext.HasComponent<O>() && !m_SelectionContext.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryTwoName.c_str()))
			{
				m_SelectionContext.AddComponent<O>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
	
	void SceneHierarchyPanel::DisplayAddScriptComponent(const std::string& name)
	{

		if (ImGui::MenuItem(name.c_str()))
		{
			m_SelectionContext.AddComponent<ScriptComponent>();
			ImGui::CloseCurrentPopup();
		}
	}
	
}
