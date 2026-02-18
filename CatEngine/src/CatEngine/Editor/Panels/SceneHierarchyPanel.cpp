#include <cepch.h>
#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Editor/ImGui/CEImGui.h"

#include "CatEngine/Scripting/ScriptEngine.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"

#include "CatEngine/AssetManager/AssetManager.h"

#include "CatEngine/Project/Project.h"



namespace CatEngine
{
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{
		SetSceneContext(scene);
	}
	void SceneHierarchyPanel::SetSceneContext(const Ref<Scene>& scene)
	{
		m_SelectionContext = {};
		m_Context = scene;

	}
	void SceneHierarchyPanel::OnImGuiRender(bool& isOpen)
	{
		ImGui::Begin("Hierarchy");
        {
            ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX
                | ImGuiTableFlags_BordersInnerV
                | ImGuiTableFlags_ScrollY;

            // Name, Visibility, Lock
            const int numberOfCols = 3;
            if (ImGui::BeginTable("##SceneHierarchy-Table", numberOfCols, tableFlags, ImVec2(ImGui::GetContentRegionAvail())))
            {

                ImGui::TableSetupColumn("Label");
                ImGui::TableSetupColumn("Visibility", ImGuiTableColumnFlags_WidthFixed, 25.0f);
                ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed, 25.0f);

                // Headers
                {
                    ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
                    for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        const char* column_name = ImGui::TableGetColumnName(column);
                        ImGui::TableHeader(column_name);
                    }
                }

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);

                {
                    for (auto entity : m_Context->GetReg().view<entt::entity>())
                    {
                        DrawEntityNode({ entity, m_Context.get() });
                    }
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Spacing();
                ImGui::Dummy(ImVec2(0, 50.0f));

                if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
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

                ImGui::EndTable();
            }
        }
		ImGui::End();

		ImGui::Begin("Inspector");
		if (m_SelectionContext)
		{
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			DrawComponents(m_SelectionContext);

			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (((float)contentRegionAvailable.x - ((float)contentRegionAvailable.x / 2.f)) * .5f));

            bool Components2DAvaliable = ((!m_SelectionContext.HasComponent<SpriteRendererComponent>() && !m_SelectionContext.HasComponent<CircleRendererComponent>()) || !m_SelectionContext.HasComponent<Rigidbody2DComponent>() || (!m_SelectionContext.HasComponent<BoxCollider2DComponent>() && !m_SelectionContext.HasComponent<CircleCollider2DComponent>()));

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
						DisplayAddComponentEntries<SpriteRendererComponent, CircleRendererComponent>("Sprite Renderer Component", "Circle Renderer Component");
						DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
						DisplayAddComponentEntries<BoxCollider2DComponent, CircleCollider2DComponent>("Box Collider 2D", "Circle Collider 2D");
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
        ImGui::PushID(std::to_string(entity.GetUUID()).c_str());
		const char* name = "Unnamed Entity";
		if (entity.HasComponent<NameComponent>())
			name = entity.GetComponent<NameComponent>().Name.c_str();

		const float edgeOffset = 4.0f;
		const float rowHeight = 21.0f;

        // ImGui item height tweaks
        auto* window = ImGui::GetCurrentWindow();
        window->DC.CurrLineSize.y = rowHeight;

		ImGui::TableNextRow(0, rowHeight);

		// Label column
		//-------------

		ImGui::TableNextColumn();

        const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
            rowAreaMin.y + rowHeight };

        const bool isSelected = m_SelectionContext == entity;

        ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        const std::string strID = std::format("{0}", name, (uint64_t)entity.GetUUID());
		
        const bool opened = ImGui::TreeNodeEx(strID.c_str(), flags);

        if (ImGui::BeginDragDropSource())
        {
            UUID entityID = entity.GetUUID();
            ImGui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", &entityID, sizeof(entityID));
            ImGui::EndDragDropSource();
        }

        if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
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

        // Visibility column

        ImGui::TableSetColumnIndex(1);

        bool visible = true;
        {
            float columnWidth = ImGui::GetColumnWidth();
            float iconSize    = ImGui::GetFrameHeight();

            float cursorX = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

            ImGui::Checkbox("##Visible", &visible);
        }

        // Lock column

        ImGui::TableSetColumnIndex(2);

        bool lock = false;
        {
            float columnWidth = ImGui::GetColumnWidth();
            float iconSize    = ImGui::GetFrameHeight(); // square icon

            float cursorX = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

            ImGui::Checkbox("##lock", &lock);
        }
        ImGui::PopID();
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
			float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
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
            if (ImGui::BeginTable("##TAGLAYER", 2))
            {
                ImGui::TableSetupColumn("TAG", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("LAYER", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                auto& tag = selection.GetComponent<TagComponent>().Tag;

                ImGui::PushItemWidth(-1.0f);
                char tagBuffer[256];
                memset(tagBuffer, 0, sizeof(tagBuffer));
                strncpy(tagBuffer, tag.c_str(), 256);
                ImGui::Text("Tag");
                ImGui::SameLine();
                if (ImGui::InputText("##T", tagBuffer, sizeof(tagBuffer)))
                {
                    tag = std::string(tagBuffer);
                }
                ImGui::PopItemWidth();

                ImGui::TableSetColumnIndex(1);
                auto& layer = selection.GetComponent<LayerComponent>().Layer;

                ImGui::PushItemWidth(-1.0f);
                char layerBuffer[256];
                memset(layerBuffer, 0, sizeof(layerBuffer));
                strncpy(layerBuffer, layer.c_str(), 256);
                ImGui::Text("Layer");
                ImGui::SameLine();
                if (ImGui::InputText("##L", layerBuffer, sizeof(layerBuffer)))
                {
                    layer = std::string(layerBuffer);
                }
                ImGui::PopItemWidth();


                ImGui::EndTable();
            }

            if (ImGui::BeginTable("##NAME", 1))
            {
                ImGui::TableSetupColumn("NAME", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushItemWidth(-1.0f);
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

                ImGui::PopItemWidth();


                ImGui::EndTable();
            }

		}

		DrawComponent<TransformComponent>("Transform", selection, [](auto& component) {
			ImGuiDraw::Vec3("Position", component.Position, 0.1f, 0.0f, 0.0f);
			glm::vec3 rotation = glm::degrees(component.Rotation);
			ImGuiDraw::Vec3("Rotation", rotation, 0.1f, 0.0f, 0.0f);
			component.Rotation = glm::radians(rotation);
			ImGuiDraw::Vec3("Scale", component.Scale, 0.1f, 0.0f, 0.0f);
		}, false);
		 
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", selection, [](auto& component) 
		{
			ImGuiDraw::ColorEdit4("Color", component.Color);
		
            std::string label = "None";
            AssetHandle handle = component.Texture;
            if (handle != 0)
            {
                if (AssetManager::IsAssetHandleValid(handle) && AssetManager::GetAssetType(handle) == AssetType::Texture2D)
                {
                    const auto& metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);
                    std::string fileName = metaData.FilePath.filename().string();
                    size_t extensionLen = metaData.FilePath.extension().string().length();
                    fileName.erase(fileName.length() - extensionLen);
                    label = fileName;
                }
            }

            ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
            buttonLabelSize.x = glm::max<float>(100.0f, buttonLabelSize.x);
            ImGui::Button(label.c_str(), buttonLabelSize);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM"))
				{
					AssetHandle handle = *(AssetHandle*)payload->Data;
                    const AssetType type = AssetManager::GetAssetType(handle);

                    if (type == AssetType::Texture2D)
                    {
                        component.Texture = handle;
                    }
                    else
                    {
                        CE_API_WARN("Cannot explicitly convert {0} to Texture2D", AssetTypeToString(type));
                    }
                }
				ImGui::EndDragDropTarget();
			}
            if (strncmp(label.c_str(), "None", 4) != 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("X"))
                {
                    component.Texture = 0;
                }
            }



			ImGuiDraw::Vec1("Tiling Factor", component.TilingFactor, 0.1f, 0.f, 100.f);
		});
		
        DrawComponent<CircleRendererComponent>("Circle Renderer", selection, [](auto& component)
		{
			ImGuiDraw::ColorEdit4("Color", component.Color);

			ImGuiDraw::Vec1("Thickness", component.Thickness, 0.025f, 0.f, 1.f);
			ImGuiDraw::Vec1("Fade", component.Fade, 0.00025f, 0.f, 1.f);
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
            ImGuiDraw::CheckBox("Primary Camera", primary);

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspectice)
			{
				auto vertFov = glm::degrees(camera.GetPerspectiveVerticalFov());
				if (ImGuiDraw::Vec1("Vertical FOV", vertFov))
					camera.SetPerspectiveVerticalFov(glm::radians(vertFov));

				auto nearClip = camera.GetPerspectiveNearClip();
				if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f, 0.f))
					camera.SetPerspectiveNearClip(nearClip);

				auto farClip = camera.GetPerspectiveFarClip();
				if (ImGuiDraw::Vec1("Far Clip", farClip, .01f, 0.f))
					camera.SetPerspectiveFarClip(farClip);
			}

			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
			{
				auto size = camera.GetOrthographicSize();
				if (ImGuiDraw::Vec1("FOV", size))
					camera.SetOrthographicSize(size);

				auto nearClip = camera.GetOrthographicNearClip();
				if (ImGuiDraw::Vec1("Near Clip", nearClip, .01f, 0.f))
					camera.SetOrthographicNearClip(nearClip);

				auto farClip = camera.GetOrthographicFarClip();
				if (ImGuiDraw::Vec1("Far Clip", farClip, .01f, 0.f))
					camera.SetOrthographicFarClip(farClip);
			}

            auto& fixed = component.FixedAspectRatio;
            ImGuiDraw::CheckBox("Fixed Aspect Ratio", fixed);
            
		});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", selection, [](auto& component)
			{
				const char* bodyTypeString[] = { "Static", "Kinematic", "Dynamic" };
				const char* currentBodyTypeString = bodyTypeString[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 3; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeString[i];
						if (ImGui::Selectable(bodyTypeString[i], &isSelected))
						{
							currentBodyTypeString = bodyTypeString[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();

					}
					ImGui::EndCombo();
				}

                ImGuiDraw::CheckBox("Fixed Rotation", component.FixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>("BoxCollider 2D", selection, [](auto& component)
			{
				glm::vec2& offset = component.Offset;
				glm::vec2& size = component.Size;

				float& density = component.Density;
				float& friction = component.Friction;
				float& restitution = component.Restitution;
				float& restitutionThreshold = component.RestitutionThreshold;

				ImGuiDraw::Vec2("Offset", component.Offset);
				ImGuiDraw::Vec2("Size", component.Size, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Rotation", component.Rotation, 0.025f, 0, 0);
				ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds);
				ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold, 0.025f, 0.0001f, 100000.f);
			});

		DrawComponent<CircleCollider2DComponent>("CircleCollider 2D", selection, [](auto& component)
			{
				glm::vec2& offset = component.Offset;
				float& size = component.Radius;

				float& density = component.Density;
				float& friction = component.Friction;
				float& restitution = component.Restitution;
				float& restitutionThreshold = component.RestitutionThreshold;

				ImGuiDraw::Vec2("Offset", component.Offset);
				ImGuiDraw::Vec1("Radius", component.Radius, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::CheckBox("Show Collider", component.ShowColliderBounds);
				ImGuiDraw::Vec1("Density", component.Density, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Friction", component.Friction, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Restitution", component.Restitution, 0.025f, 0.0001f, 100000.f);
				ImGuiDraw::Vec1("Restitution Threshold", component.RestitutionThreshold, 0.025f, 0.0001f, 100000.f);
			});

		DrawComponent<ScriptComponent>("Script", selection, [selection, this](auto& component) mutable
        {
            auto& sc = component;
            auto scriptEngine = ScriptEngine::GetMutable();
			const auto& entityClasses = scriptEngine.GetScriptClasses();

            std::vector<const char*> classes = {""};
            for (auto& [_, scriptClass] : entityClasses)
            {
                classes.push_back(scriptClass->GetClassName());
            }

            if (ImGuiDraw::Combo("Class", sc.SelectedScript, classes))
            {
                sc.ClassName = sc.SelectedScript != 0 ? classes.at(sc.SelectedScript) : "";
            }


            if (!sc.Loaded && !sc.ClassName.empty())
            {
                int i = 0;
                for (auto& [_, scriptClass] : entityClasses)
                {
                    i++;
                    std::string scriptName = scriptClass->GetClassName();
                    if (sc.ClassName == scriptName)
                    {
                        sc.SelectedScript = i;
                    }
                }
            }

            sc.Loaded = true;


			bool scriptClassExists = scriptEngine.ScriptClassExists(sc.ClassName.c_str());
			bool sceneRunning = m_Context->IsRunning();
			if (sceneRunning)
			{
				Ref<ScriptInstance> scriptInstance = scriptEngine.GetEntityScriptInstance(selection.GetUUID());
				if (scriptInstance)
				{
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();


					for (const auto& [uuid, field] : fields)
                    {
                        switch (field.Type)
                        {
                            case DataType::Short:
                            {
                                int16_t data = scriptInstance->GetFieldData<int16_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::UShort:
                            {
                                uint16_t data = scriptInstance->GetFieldData<uint16_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Int:
                            {
                                int32_t data = scriptInstance->GetFieldData<int32_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::UInt:
                            {
                                uint32_t data = scriptInstance->GetFieldData<uint32_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Long:
                            {
                                int64_t data = scriptInstance->GetFieldData<int64_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::ULong:
                            {
                                uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
                                if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    scriptInstance->SetFieldData(field.Name, &data);
                                break;
                            }
                            case DataType::Float:
                            {
                                float data = scriptInstance->GetFieldData<float>(field.Name);
                                if (ImGuiDraw::Vec1(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Double:
                            {
                                double data = scriptInstance->GetFieldData<double>(field.Name);
                                if (ImGuiDraw::Vec1(field.Name, (float&)data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Bool:
                            {
                                bool data = scriptInstance->GetFieldData<bool>(field.Name);
                                if (ImGuiDraw::CheckBox(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Vector2:
                            {
                                glm::vec2 data = scriptInstance->GetFieldData<glm::vec2>(field.Name);
                                if (ImGuiDraw::Vec2(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Vector3:
                            {
                                glm::vec3 data = scriptInstance->GetFieldData<glm::vec3>(field.Name);
                                if (ImGuiDraw::Vec3(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Vector4:
                            {
                                glm::vec4 data = scriptInstance->GetFieldData<glm::vec4>(field.Name);
                                if (ImGuiDraw::Vec4(field.Name, data))
                                    scriptInstance->SetFieldData(field.Name, data);
                                break;
                            }
                            case DataType::Entity:
                            {
                                uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
                                std::string labelName = m_Context->GetEntityByUUID(data).GetName();
                                labelName = labelName.empty() ? "(empty)" : labelName;
                                labelName.append(" (Entity)");
                                ImGuiDraw::DataField(field.Name, labelName);
                                break;
                            }
                            case DataType::Texture2D:
                            {
                                uint64_t data = scriptInstance->GetFieldData<uint64_t>(field.Name);
                                std::string labelName =
                                    Project::GetActive()->GetEditorAssetManager()->GetMetaData(data).AssetName;
                                labelName = labelName.empty() ? "(empty)" : labelName;
                                labelName.append(" (Texture2D)");
                                ImGuiDraw::DataField(field.Name, labelName);
                                break;
                            }
                        }

                    }
                }
            }
            else // Scene isn't running
			{
                if(scriptClassExists)
                {
					Ref<ScriptClass> scriptClass = scriptEngine.GetScriptClass(component.ClassName.c_str());
					const auto& fields = scriptClass->GetFields();

					auto& scriptFields = scriptEngine.GetScriptFieldMap(selection);

					for (const auto& [uuid, field] : fields)
					{
						if (scriptFields.find(uuid) != scriptFields.end())
						{
							// Display control to set it
							
                            switch (field.Type)
                            {
                                case DataType::Short:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    int16_t data = scriptFieldInstance.GetValue<int16_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::UShort:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    uint16_t data = scriptFieldInstance.GetValue<uint16_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Int:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    int32_t data = scriptFieldInstance.GetValue<int32_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::UInt:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    uint32_t data = scriptFieldInstance.GetValue<uint32_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Long:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    int64_t data = scriptFieldInstance.GetValue<int64_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::ULong:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    uint64_t data = scriptFieldInstance.GetValue<uint64_t>();
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Float:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    float data = scriptFieldInstance.GetValue<float>();
                                    if (ImGuiDraw::Vec1(field.Name, data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Double:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    double data = scriptFieldInstance.GetValue<double>();
                                    if (ImGuiDraw::Vec1(field.Name, (float&)data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Bool:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    bool data = scriptFieldInstance.GetValue<bool>();
                                    if (ImGuiDraw::CheckBox(field.Name, data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Vector2:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    glm::vec2 data = scriptFieldInstance.GetValue<glm::vec2>();
                                    if (ImGuiDraw::Vec2(field.Name, data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Vector3:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    glm::vec3 data = scriptFieldInstance.GetValue<glm::vec3>();
                                    if (ImGuiDraw::Vec3(field.Name, data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Vector4:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    glm::vec4 data = scriptFieldInstance.GetValue<glm::vec4>();
                                    if (ImGuiDraw::Vec4(field.Name, data))
                                        scriptFieldInstance.SetValue(data);
                                    break;
                                }

                                case DataType::Entity:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    uint64_t data = scriptFieldInstance.GetValue<uint64_t>();

                                    std::string labelName = m_Context->GetEntityByUUID(data).GetName();
                                    labelName.append(" (Entity)");
                                    ImGuiDraw::DataField(field.Name, labelName);

                                    if (ImGui::BeginDragDropTarget())
                                    {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
                                        {
                                            UUID entityID = *(UUID*)payload->Data;
                                            scriptFieldInstance.SetValue(entityID.uuid());
                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                    break;
                                }

                                case DataType::Texture2D:
                                {
                                    ScriptFieldInstance& scriptFieldInstance = scriptFields.at(uuid);
                                    uint64_t data = scriptFieldInstance.GetValue<uint64_t>();

                                    std::string label =
                                        Project::GetActive()->GetEditorAssetManager()->GetMetaData(data).AssetName;
                                    label.append(" (Texture2D)");
                                    ImGuiDraw::DataField(field.Name, label);

                                    if (ImGui::BeginDragDropTarget())
                                    {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM"))
                                        {
                                            UUID assetHandle = *(UUID*)payload->Data;
                                            const auto& asset =
                                                Project::GetActive()->GetEditorAssetManager()->GetAsset(assetHandle);

                                            if (asset->GetType() == AssetType::Texture2D)
                                                scriptFieldInstance.SetValue(assetHandle);
                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                    break;
                                }
                            }
                        }
                        else // Script field instance isn't in the map
                        {
                            switch (field.Type)
                            {
                                case DataType::Short:
                                {
                                    int16_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::UShort:
                                {
                                    uint16_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Int:
                                {
                                    int32_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::UInt:
                                {
                                    uint32_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Long:
                                {
                                    int64_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::ULong:
                                {
                                    uint64_t data = 0;
                                    if (ImGuiDraw::Int1(field.Name, (int&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Float:
                                {
                                    float data = 0.0f;
                                    if (ImGuiDraw::Vec1(field.Name, data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Double:
                                {
                                    double data = 0.0;
                                    if (ImGuiDraw::Vec1(field.Name, (float&)data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Bool:
                                {
                                    bool data = false;
                                    if (ImGuiDraw::CheckBox(field.Name, data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Vector2:
                                {
                                    glm::vec2 data(0.0f);
                                    if (ImGuiDraw::Vec2(field.Name, data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Vector3:
                                {
                                    glm::vec3 data(0.0f);
                                    if (ImGuiDraw::Vec3(field.Name, data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Vector4:
                                {
                                    glm::vec4 data(0.0f);
                                    if (ImGuiDraw::Vec4(field.Name, data))
                                    {
                                        ScriptFieldInstance& sfi = scriptFields[uuid];
                                        sfi.Field = field;
                                        sfi.SetValue(data);
                                    }
                                    break;
                                }
                                case DataType::Entity:
                                {
                                    ImGuiDraw::DataField(field.Name, "Entity (empty)");
                                    if (ImGui::BeginDragDropTarget())
                                    {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM"))
                                        {
                                            UUID entityID = *(UUID*)payload->Data;
                                            ScriptFieldInstance& sfi = scriptFields[uuid];
                                            sfi.Field = field;
                                            sfi.SetValue(entityID.uuid());

                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                    ImGui::PopItemWidth();
                                    break;
                                }
                                case DataType::Texture2D:
                                {
                                    ImGuiDraw::DataField(field.Name, "Texture2D (empty)");
                                    if (ImGui::BeginDragDropTarget())
                                    {
                                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_ITEM"))
                                        {
                                            UUID assetID = *(UUID*)payload->Data;
                                            const auto& asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(assetID);
                                            if (asset->GetType() == AssetType::Texture2D)
                                            {
                                                ScriptFieldInstance& sfi = scriptFields[uuid];
                                                sfi.Field = field;
                                                sfi.SetValue(assetID.uuid());
                                            }
                                        }
                                        ImGui::EndDragDropTarget();
                                    }
                                    break;
                                }
                            }
                        }

                    }
                }


            }
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
        if (!m_SelectionContext.HasComponent<ScriptComponent>())
        {
            if (ImGui::MenuItem(name.c_str()))
            {
                m_SelectionContext.AddComponent<ScriptComponent>();
                ImGui::CloseCurrentPopup();
            }
        }
	}
	

}
