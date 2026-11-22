#include "SceneHierarchyPanel.h"
#include <cepch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/Application.h"
#include "CatEngine/Core/UUID.h"
#include "CatEngine/Editor/IconManager.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Scene/Components/Components.h"

#include "CatEngine/Editor/ImGui/CEImGui.h"

#include "CatEngine/Scene/Components/Required/RelationshipComponent.h"
#include "CatEngine/Scripting/ScriptEngine.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"

#include "CatEngine/AssetManager/AssetManager.h"

#include "CatEngine/Project/Project.h"

#include "DrawComponents.h"

namespace CatEngine {
SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene) {
  SetSceneContext(scene);
}
void SceneHierarchyPanel::SetSceneContext(const Ref<Scene> &scene) {
  m_SelectionContext = {};
  m_Context = scene;
}
void SceneHierarchyPanel::OnImGuiRender(bool &isOpen) {
  ImGui::Begin("Hierarchy");
  {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_SpanAvailWidth |
                               ImGuiTreeNodeFlags_DefaultOpen;

    auto &activeProject = Project::GetActive();


    const Asset::MetaData &sceneMetaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(m_Context->m_Handle);
    bool is_Prefab = sceneMetaData.Type == AssetType::Prefab;
    std::string projName = Project::GetConfig().Name;

    if (ImGui::TreeNodeEx(projName.c_str(), flags)) {

      // Name, Visibility, Lock
      const std::vector<AssetHandle> openedScenes = Project::GetActive()->GetOpenScenes();
      for (auto it = openedScenes.rbegin(); it != openedScenes.rend(); it++) {
        UUID handle = *it;
        if (handle == m_Context->m_Handle)
          continue;
        const Asset::MetaData &metaData = activeProject->GetEditorAssetManager()->GetMetaData(handle);
        if(ImGui::TreeNodeEx(metaData.AssetName.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf))
          ImGui::TreePop();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
          Application::Get().OpenSceneFromHandle(handle);
        }
      }

      ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX |
                                   ImGuiTableFlags_BordersInnerV |
                                   ImGuiTableFlags_ScrollY;

      std::string sceneName = sceneMetaData.AssetName;
      if (is_Prefab) {
        sceneName = sceneName + " (Scene)";
      }
      if (ImGui::TreeNodeEx(sceneName.c_str(), flags)) {
        const int numberOfCols = 3;
        if (ImGui::BeginTable("##SceneHierarchy-Table", numberOfCols, tableFlags,
                              ImVec2(ImGui::GetContentRegionAvail()))) {

          ImGui::TableSetupColumn("Label");
          ImGui::TableSetupColumn("Visibility", ImGuiTableColumnFlags_WidthFixed,
                                  25.0f);
          ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed,
                                  25.0f);
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);

          {
            for (auto entity : m_Context->GetRootEntities()) {
              Entity e = m_Context->GetEntityByUUID(entity);
              auto &rel = e.GetComponent<RelationshipComponent>();
              if (rel.Parent == 0) { // Root Node
                DrawEntityNode(e, is_Prefab);
              }
            }
          }

          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Spacing();
          ImGui::Dummy(ImGui::GetContentRegionAvail());

          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
              UUID droppedEntityID = *(UUID *)payload->Data;
              Entity droppedEntity = m_Context->GetEntityByUUID(droppedEntityID);

              if (droppedEntity) {
                // Dropped into empty space = Make it a root-level entity (Parent
                // = 0)
                m_Context->ReparentEntity(droppedEntity, {});
              }
            }
            ImGui::EndDragDropTarget();
          }

          if (ImGui::BeginPopupContextWindow(
            nullptr, ImGuiPopupFlags_MouseButtonRight |
            ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::BeginMenu("Create")) {
              if (ImGui::MenuItem("Create Empty GameObject")) {
                const auto &entity = m_Context->CreateEntity("GameObject");
                Ref<InspectorPanel> inspector =
                  PanelManager::Get().GetPanel<InspectorPanel>(
                    "InspectorPanel");
                inspector->SetEntitySelectionContext(entity);
                ImGui::CloseCurrentPopup();
              }
              if (ImGui::MenuItem("Create Camera")) {
                const auto &entity = m_Context->CreateEntity("Camera");
                entity.AddComponent<CameraComponent>().Primary = false;
                Ref<InspectorPanel> inspector =
                  PanelManager::Get().GetPanel<InspectorPanel>(
                    "InspectorPanel");
                inspector->SetEntitySelectionContext(entity);
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndMenu();
            }
            ImGui::EndPopup();
          }

          ImGui::EndTable();

          ImGui::TreePop();
        }

      }
      ImGui::TreePop();
    }
  }
  ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity, bool isPrefabRoot) {
  if (!entity || !m_Context->GetReg().valid(entity))
    return;

  ImGui::PushID(std::to_string(entity.GetUUID()).c_str());
  std::string name = "Unnamed Entity";
  name = entity.GetComponent<NameComponent>().Name.c_str();

  if (isPrefabRoot) {
    name = name + " (Root)";
  }

  auto &relationship = entity.GetComponent<RelationshipComponent>();

  const float edgeOffset = 4.0f;
  const float rowHeight = 21.0f;

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth |
                             ImGuiTreeNodeFlags_FramePadding;

  // If it has no children, make it a leaf node so it doesn't show an arrow
  if (relationship.Children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  // Highlight if selected
  if (m_SelectionContext == entity) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  // ImGui item height tweaks
  auto *window = ImGui::GetCurrentWindow();
  window->DC.CurrLineSize.y = rowHeight;

  ImGui::TableNextRow(0, rowHeight);

  // Label column
  //-------------

  ImGui::TableNextColumn();

  const ImVec2 rowAreaMin =
      ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
  const ImVec2 rowAreaMax = {
      ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(),
                                ImGui::TableGetColumnCount() - 1)
              .Max.x -
          20,
      rowAreaMin.y + rowHeight};

  const std::string strID = name;

  const bool opened = ImGui::TreeNodeEx(strID.c_str(), flags);

  if (ImGui::BeginDragDropSource()) {
    UUID entityID = entity.GetUUID();
    ImGui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", &entityID,
                              sizeof(entityID));
    ImGui::TextUnformatted(name.c_str());
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
      UUID droppedEntityID = *(UUID *)payload->Data;
      Entity droppedEntity = m_Context->GetEntityByUUID(droppedEntityID);

      if (droppedEntity && droppedEntity != entity) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemRectMin = ImGui::GetItemRectMin();
        ImVec2 itemRectMax = ImGui::GetItemRectMax();
        float itemHeight = itemRectMax.y - itemRectMin.y;

        float relativeY = mousePos.y - itemRectMin.y;

        // Wased thresholds: Top 35% = Before, Bottom 35% = After, Middle 30% =
        // Reparent
        if (relativeY < itemHeight * 0.35f) {
          // Drop Above (Sibling Reorder - Before)
          m_Context->MoveEntityBefore(droppedEntity, entity);
        } else if (relativeY > itemHeight * 0.65f) {
          // Drop Below (Sibling Reorder - After)
          m_Context->MoveEntityAfter(droppedEntity, entity);
        } else {
          // Drop Directly On (Reparent as Child)
          m_Context->ReparentEntity(droppedEntity, entity);
        }
      }
    }

    // Optional: Draw visual indicator lines so you can see where it's dropping
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 itemRectMin = ImGui::GetItemRectMin();
    ImVec2 itemRectMax = ImGui::GetItemRectMax();
    float itemHeight = itemRectMax.y - itemRectMin.y;
    float relativeY = mousePos.y - itemRectMin.y;

    auto *drawList = ImGui::GetWindowDrawList();
    if (relativeY < itemHeight * 0.35f) {
      // Draw line above item
      drawList->AddLine(itemRectMin, ImVec2(itemRectMax.x, itemRectMin.y),
                        IM_COL32(0, 120, 255, 255), 2.0f);
    } else if (relativeY > itemHeight * 0.65f) {
      // Draw line below item
      drawList->AddLine(ImVec2(itemRectMin.x, itemRectMax.y), itemRectMax,
                        IM_COL32(0, 120, 255, 255), 2.0f);
    }

    ImGui::EndDragDropTarget();
  }

  if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
    Ref<InspectorPanel> inspector =
        PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
    inspector->SetEntitySelectionContext(entity);
    m_SelectionContext = entity;
  }

  bool entityDeleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete"))
      entityDeleted = true;

    ImGui::EndPopup();
  }

  // Visibility column

  ImGui::TableSetColumnIndex(1);

  auto &stateComponent = entity.GetComponent<ObjectStateComponent>();

  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight();

    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);
    Ref<Texture2D> icon = stateComponent.SelfActive ? 
      IconManager::GetIcon("AssetVisibility-Visible") :
      IconManager::GetIcon("AssetVisibility-Invisible");
    if (ImGui::ImageButton("##ASSET_VISIBILITY_BUTTONE", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1))) {
      bool isActive = !stateComponent.SelfActive;
      m_Context->SetEntityActive(entity, isActive);
    }
  }

  // Lock column

  ImGui::TableSetColumnIndex(2);

  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight(); // square icon

    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);
    Ref<Texture2D> icon = stateComponent.isLocked ? 
      IconManager::GetIcon("AssetLock-Locked") :
      IconManager::GetIcon("AssetLock-Unlocked");
    if (ImGui::ImageButton("##ASSET_LOCK_BUTTON", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2(0, 1), ImVec2(1, 0))) {
      stateComponent.isLocked = !stateComponent.isLocked;
    }
  }
  if (opened) {
    for (auto childUUID : relationship.Children) {
      Entity childEntity = m_Context->GetEntityByUUID(childUUID);
      if (childEntity) {
        DrawEntityNode(childEntity);
      }
    }
    ImGui::TreePop();
  }

  if (entityDeleted) {
    if (m_SelectionContext == entity) {
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->SetEntitySelectionContext({});
      m_SelectionContext = {};
    }
    m_Context->DeleteEntity(entity);
  }


  ImGui::PopID();
}

} // namespace CatEngine
