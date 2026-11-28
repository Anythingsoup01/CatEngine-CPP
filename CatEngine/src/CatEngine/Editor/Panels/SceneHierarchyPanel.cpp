#include "cepch.h" // IWYU pragma: keep
#include "SceneHierarchyPanel.h"

#include "CatEngine/Core/Application.h"

#include "CatEngine/Editor/IconManager.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"

#include "CatEngine/Project/Project.h"


#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace CatEngine {
SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> &scene) {
  SetSceneContext(scene);
}
void SceneHierarchyPanel::SetSceneContext(const Ref<Scene> &scene) {
  m_SelectionContext = {};
  m_Context = scene;
}

void SceneHierarchyPanel::OnImGuiRender(bool &isOpen) {
  ImGui::Begin("Hierarchy", &isOpen);
  ImGuiIO &io = ImGui::GetIO();
  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) || io.MouseWheel != 0.0f)
      ImGui::FocusWindow(ImGui::GetCurrentWindow());
  }
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth |
                             ImGuiTreeNodeFlags_DefaultOpen;

  auto &activeProject = Project::GetActive();
  auto assetManager = activeProject->GetEditorAssetManager();

  const Asset::MetaData &sceneMetaData =
      assetManager->GetMetaData(m_Context->m_Handle);
  bool is_Prefab = sceneMetaData.Type == AssetType::Prefab;
  std::string projName = Project::GetConfig().Name;

  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.19f, 0.22f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                        ImVec4(0.23f, 0.25f, 0.29f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                        ImVec4(0.21f, 0.23f, 0.27f, 1.0f));

  if (ImGui::TreeNodeEx(projName.c_str(), flags)) {
    ImGui::PopStyleColor(3);

    // Collect scene handles to close after iteration finishes
    std::vector<AssetHandle> scenesToClose;

    const std::vector<AssetHandle> openedScenes =
        activeProject->GetOpenScenes();
    for (auto it = openedScenes.rbegin(); it != openedScenes.rend(); ++it) {
      AssetHandle handle = *it;
      if (handle == m_Context->m_Handle)
        continue;

      const Asset::MetaData &metaData = assetManager->GetMetaData(handle);
      ImGui::PushID(static_cast<uint64_t>(handle));

      // Muted slate tint for background/inactive open scenes
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.68f, 0.71f, 0.76f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_Header,
                            ImVec4(0.16f, 0.17f, 0.20f, 0.65f));
      ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                            ImVec4(0.22f, 0.24f, 0.28f, 0.85f));
      ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                            ImVec4(0.20f, 0.22f, 0.26f, 0.95f));

      ImGuiTreeNodeFlags openSceneFlags =
          ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf |
          ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{6.0f, 3.0f});
      std::string label = metaData.AssetName + "##OpenedScene";
      if (ImGui::TreeNodeEx(label.c_str(), openSceneFlags, "%s",
                            metaData.AssetName.c_str())) {
        ImGui::TreePop();
      }
      ImGui::PopStyleVar();

      if (ImGui::IsItemHovered() &&
          ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        Application::Get().OpenSceneFromHandle(handle);
      }

      // Right-aligned close button ('x') to remove from opened scenes
      const float closeBtnSize = ImGui::GetFrameHeight();
      const float rightEdge = ImGui::GetWindowContentRegionMax().x;
      ImGui::SameLine(rightEdge - closeBtnSize - 4.0f);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(0.35f, 0.20f, 0.22f, 0.85f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            ImVec4(0.48f, 0.22f, 0.25f, 1.0f));
      if (ImGui::SmallButton("x")) {
        scenesToClose.push_back(handle);
      }
      ImGui::PopStyleColor(3);

      ImGui::PopStyleColor(4);
      ImGui::PopID();
    }

    for (AssetHandle closedHandle : scenesToClose) {
      activeProject->CloseScene(closedHandle);
    }

    ImGui::Spacing();

    // Active / current scene header styled more prominently like Unity's active
    // scene
    std::string sceneName = sceneMetaData.AssetName;
    if (is_Prefab) {
      sceneName += " (Prefab Scene)";
    }

    ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX |
                                 ImGuiTableFlags_BordersInnerV |
                                 ImGuiTableFlags_ScrollY;

    ImGui::PushStyleColor(ImGuiCol_Text,
                          is_Prefab ? ImVec4(0.55f, 0.78f, 1.0f, 1.0f)
                                    : ImVec4(0.93f, 0.95f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header,
                          is_Prefab ? ImVec4(0.16f, 0.27f, 0.43f, 0.90f)
                                    : ImVec4(0.22f, 0.25f, 0.31f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                          is_Prefab ? ImVec4(0.20f, 0.33f, 0.52f, 1.0f)
                                    : ImVec4(0.27f, 0.31f, 0.38f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                          is_Prefab ? ImVec4(0.18f, 0.30f, 0.48f, 1.0f)
                                    : ImVec4(0.25f, 0.29f, 0.36f, 1.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{6.0f, 4.0f});
    bool sceneOpen = ImGui::TreeNodeEx(sceneName.c_str(), flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (sceneOpen) {
      const int numberOfCols = 3;
      if (ImGui::BeginTable("##SceneHierarchy-Table", numberOfCols, tableFlags,
                            ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("Label");
        ImGui::TableSetupColumn("Visibility", ImGuiTableColumnFlags_WidthFixed,
                                25.0f);
        ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed,
                                25.0f);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        for (auto entity : m_Context->GetRootEntities()) {
          Entity e = m_Context->GetEntityByUUID(entity);
          auto &rel = e.GetComponent<RelationshipComponent>();
          if (rel.Parent == 0) {
            DrawEntityNode(e, is_Prefab);
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
              m_Context->ReparentEntity(droppedEntity, {});
            }
          }
          ImGui::EndDragDropTarget();
        }
        DrawContextMenu({});
        ImGui::EndTable();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  } else {
    ImGui::PopStyleColor(3);
  }
  ImGui::End();
}

void SceneHierarchyPanel::SetSelectedEntity(Entity entity) {
  if (m_SelectionContext.IsParentOf(entity)) {
    m_SelectionContext = entity;
  } else {
    m_SelectionContext = entity.GetRootParent();
  }
}


void SceneHierarchyPanel::DrawEntityNode(Entity entity, bool isPrefabRoot) {
  if (!entity || !m_Context->GetReg().valid(entity))
    return;

  ImGui::PushID(static_cast<uint64_t>(entity.GetUUID()));

  std::string name = "Unnamed Entity";
  if (entity.HasComponent<NameComponent>()) {
    name = entity.GetComponent<NameComponent>().Name;
  }

  if (isPrefabRoot) {
    name += " (Root)";
  }

  auto &relationship = entity.GetComponent<RelationshipComponent>();
  auto &stateComponent = entity.GetComponent<ObjectStateComponent>();
  const bool hasPrefabLink = entity.HasComponent<PrefabLinkComponent>() || isPrefabRoot;
  const bool isSelected = (m_SelectionContext == entity);
  const bool isActiveInHierarchy = stateComponent.Active;

  const float rowHeight = 21.0f;

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth |
                             ImGuiTreeNodeFlags_FramePadding;

  if (relationship.Children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  if (isSelected) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  if (isPrefabRoot || entity.IsParentOf(m_SelectionContext)) {
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  ImVec4 textColor;
  if (hasPrefabLink) {
    textColor = isActiveInHierarchy
      ? ImVec4(0.55f, 0.78f, 1.0f, 1.0f)
      : ImVec4(0.36f, 0.50f, 0.66f, 0.80f);
  } else if (!isActiveInHierarchy) {
    textColor = ImVec4(0.50f, 0.53f, 0.58f, 0.85f);
  } else {
    textColor = ImVec4(0.92f, 0.94f, 0.97f, 1.0f);
  }

  ImVec4 headerSelected = hasPrefabLink
    ? ImVec4(0.18f, 0.34f, 0.56f, 0.95f)
    : ImVec4(0.24f, 0.38f, 0.58f, 0.90f);

  ImVec4 headerHovered = ImVec4(0.23f, 0.25f, 0.30f, 0.75f);
  ImVec4 headerActive  = ImVec4(0.20f, 0.28f, 0.40f, 0.90f);

  auto *window = ImGui::GetCurrentWindow();
  window->DC.CurrLineSize.y = rowHeight;

  ImGui::TableNextRow(0, rowHeight);

  // Label column
  ImGui::TableNextColumn();

  ImGui::PushStyleColor(ImGuiCol_Text, textColor);
  ImGui::PushStyleColor(ImGuiCol_Header, isSelected ? headerSelected : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, isSelected ? headerSelected : headerHovered);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, headerActive);

  const bool shouldForceOpen = isPrefabRoot || entity.IsParentOf(m_SelectionContext);
  if (shouldForceOpen) {
    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
  }

  const std::string strID = "##EntityNode";
  const bool opened = ImGui::TreeNodeEx(strID.c_str(), flags, "%s", name.c_str());

  ImGui::PopStyleColor(4);

  // Single-click selection when not dragging or toggling arrow
  if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    m_SelectionContext = entity;
    Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
    inspector->UpdateEntitySelection();
  }

  if (ImGui::BeginDragDropSource()) {
    UUID entityID = entity.GetUUID();
    ImGui::SetDragDropPayload("SCENE_HIERARCHY_ITEM", &entityID, sizeof(entityID));
    ImGui::TextUnformatted(name.c_str());
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
      UUID droppedEntityID = *static_cast<const UUID *>(payload->Data);
      Entity droppedEntity = m_Context->GetEntityByUUID(droppedEntityID);

      if (droppedEntity && droppedEntity != entity) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemRectMin = ImGui::GetItemRectMin();
        ImVec2 itemRectMax = ImGui::GetItemRectMax();
        float itemHeight = itemRectMax.y - itemRectMin.y;
        float relativeY = mousePos.y - itemRectMin.y;

        // Top 35% = Before, Bottom 35% = After, Middle 30% = Reparent
        if (relativeY < itemHeight * 0.35f) {
          m_Context->MoveEntityBefore(droppedEntity, entity);
        } else if (relativeY > itemHeight * 0.65f) {
          m_Context->MoveEntityAfter(droppedEntity, entity);
        } else {
          m_Context->ReparentEntity(droppedEntity, entity);
        }
      }
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 itemRectMin = ImGui::GetItemRectMin();
    ImVec2 itemRectMax = ImGui::GetItemRectMax();
    float itemHeight = itemRectMax.y - itemRectMin.y;
    float relativeY = mousePos.y - itemRectMin.y;

    auto *drawList = ImGui::GetWindowDrawList();
    if (relativeY < itemHeight * 0.35f) {
      drawList->AddLine(itemRectMin, ImVec2(itemRectMax.x, itemRectMin.y),
                        IM_COL32(86, 156, 255, 255), 2.0f);
    } else if (relativeY > itemHeight * 0.65f) {
      drawList->AddLine(ImVec2(itemRectMin.x, itemRectMax.y), itemRectMax,
                        IM_COL32(86, 156, 255, 255), 2.0f);
    }

    ImGui::EndDragDropTarget();
  }

  DrawContextMenu(entity);

  // Visibility column
  ImGui::TableSetColumnIndex(1);
  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight();
    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

    Ref<Texture2D> icon = stateComponent.SelfActive
      ? IconManager::GetIcon("AssetVisibility-Visible")
      : IconManager::GetIcon("AssetVisibility-Invisible");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.30f, 0.35f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.38f, 0.44f, 0.85f));

    if (ImGui::ImageButton("##ASSET_VISIBILITY_BUTTON",
                           (ImTextureID)((uint64_t)(icon->GetRendererID())),
                           ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1))) {
      m_Context->SetEntityActive(entity, !stateComponent.SelfActive);
    }

    ImGui::PopStyleColor(3);
  }

  // Lock column
  ImGui::TableSetColumnIndex(2);
  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight();
    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

    Ref<Texture2D> icon = stateComponent.Locked
      ? IconManager::GetIcon("AssetLock-Locked")
      : IconManager::GetIcon("AssetLock-Unlocked");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.30f, 0.35f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.38f, 0.44f, 0.85f));

    if (ImGui::ImageButton("##ASSET_LOCK_BUTTON",
                           (ImTextureID)((uint64_t)(icon->GetRendererID())),
                           ImVec2(iconSize, iconSize), ImVec2(0, 1), ImVec2(1, 0))) {
      stateComponent.Locked = !stateComponent.Locked;
    }

    ImGui::PopStyleColor(3);
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

  ImGui::PopID();
}

void SceneHierarchyPanel::DrawContextMenu(Entity hoveredEntity) {
  UUID parent = 0;
  if (hoveredEntity) {
    auto &rc = hoveredEntity.GetComponent<RelationshipComponent>();
    parent = rc.Parent;
  }

  ImGuiPopupFlags flags = ImGuiPopupFlags_MouseButtonRight;

  bool opened = false;
  if (hoveredEntity) {
    opened = ImGui::BeginPopupContextItem(nullptr, flags);
  } else {
    flags |= ImGuiPopupFlags_NoOpenOverItems;
    opened = ImGui::BeginPopupContextWindow(nullptr, flags);
  }

  if (!opened)
    return;

  if (ImGui::MenuItem("Create Empty")) {
    const auto &entity = m_Context->CreateEntity("GameObject", parent);
    if (parent) {
      Entity parentEntity = m_Context->GetEntityByUUID(parent);
      auto &rc = parentEntity.GetComponent<RelationshipComponent>();
      rc.Children.push_back(entity.GetUUID());
    }
    m_SelectionContext = entity;
    Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
    inspector->UpdateEntitySelection();
    ImGui::CloseCurrentPopup();
  }

  if (hoveredEntity) {
    if (ImGui::MenuItem("Create Empty Child")) {
      const auto &entity = m_Context->CreateEntity("GameObject", hoveredEntity.GetUUID());
      auto &newEntityRC = entity.GetComponent<RelationshipComponent>();
      auto &hoveredEntityRC =
          hoveredEntity.GetComponent<RelationshipComponent>();
      hoveredEntityRC.Children.push_back(entity.GetUUID());
      newEntityRC.Parent = hoveredEntity.GetUUID();
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();
      ImGui::CloseCurrentPopup();
    }
// TODO:
#if 0
    if (ImGui::MenuItem("Create Empty Parent")) {
      const auto &entity = m_Context->CreateEntity("GameObject", parent);
      auto &newEntityRC = entity.GetComponent<RelationshipComponent>();
      auto &hoveredEntityRC = hoveredEntity.GetComponent<RelationshipComponent>();
      newEntityRC.Children.push_back(hoveredEntity.GetUUID());
      hoveredEntityRC.Parent = entity.GetUUID();

      if (parent) {
        auto parentEntity = m_Context->GetEntityByUUID(parent);
        auto &parentRC = parentEntity.GetComponent<RelationshipComponent>();

        for (auto it = parentRC.Children.begin(); it != parentRC.Children.end(); it++) {
          if (*it == hoveredEntity.GetUUID()) {
            parentRC.Children.erase(it);
            break;
          }
        }

        parentRC.Children.push_back(entity.GetUUID());
      }
      m_Context->RegisterParents();
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->SetEntitySelectionContext(entity);
      ImGui::CloseCurrentPopup();
    }
#endif
  }

  if (ImGui::BeginMenu("2D")) {
    if (ImGui::MenuItem("Create Sprite")) {
      const auto &entity = m_Context->CreateEntity("GameObject", parent);
      entity.AddComponent<SpriteRendererComponent>();
      if (parent) {
        Entity parentEntity = m_Context->GetEntityByUUID(parent);
        auto &rc = parentEntity.GetComponent<RelationshipComponent>();
        rc.Children.push_back(entity.GetUUID());
      }
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Create Circle")) {
      const auto &entity = m_Context->CreateEntity("GameObject", parent);
      entity.AddComponent<CircleRendererComponent>();
      if (parent) {
        Entity parentEntity = m_Context->GetEntityByUUID(parent);
        auto &rc = parentEntity.GetComponent<RelationshipComponent>();
        rc.Children.push_back(entity.GetUUID());
      }
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Create Rigid Box")) {
      const auto &entity = m_Context->CreateEntity("GameObject", parent);
      entity.AddComponent<SpriteRendererComponent>();
      entity.AddComponent<Rigidbody2DComponent>();
      entity.AddComponent<BoxCollider2DComponent>();
      if (parent) {
        Entity parentEntity = m_Context->GetEntityByUUID(parent);
        auto &rc = parentEntity.GetComponent<RelationshipComponent>();
        rc.Children.push_back(entity.GetUUID());
      }
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Create Rigid Circle")) {
      const auto &entity = m_Context->CreateEntity("GameObject", parent);
      entity.AddComponent<CircleRendererComponent>();
      entity.AddComponent<Rigidbody2DComponent>();
      entity.AddComponent<CircleCollider2DComponent>();
      if (parent) {
        Entity parentEntity = m_Context->GetEntityByUUID(parent);
        auto &rc = parentEntity.GetComponent<RelationshipComponent>();
        rc.Children.push_back(entity.GetUUID());
      }
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("UI")) {
    if (ImGui::MenuItem("Create Canvas")) {
      const auto &entity = m_Context->CreateCanvas("Canvas", parent);
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Create Panel")) {
      const auto &entity = m_Context->CreatePanel(parent);
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("Create Button")) {
      const auto &entity = m_Context->CreateButton(parent);
      m_SelectionContext = entity;
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndMenu();
  }


  if (ImGui::MenuItem("Create Camera")) {
    const auto &entity = m_Context->CreateEntity("Camera");
    entity.AddComponent<CameraComponent>().Primary = false;
    if (parent) {
      Entity parentEntity = m_Context->GetEntityByUUID(parent);
      auto &rc = parentEntity.GetComponent<RelationshipComponent>();
      rc.Children.push_back(entity.GetUUID());
    }
    m_SelectionContext = entity;
    Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
    inspector->UpdateEntitySelection();
    ImGui::CloseCurrentPopup();
  }

  if (hoveredEntity) {
    if (ImGui::MenuItem("Delete")) {
      m_Context->DeleteEntity(hoveredEntity);
      if (parent) {
        auto parentEntity = m_Context->GetEntityByUUID(parent);
        auto &rc = parentEntity.GetComponent<RelationshipComponent>();
        for (auto it = rc.Children.begin(); it != rc.Children.end(); it++) {
          if (*it == hoveredEntity.GetUUID()) {
            rc.Children.erase(it);
            break;
          }
        }
      }
      m_SelectionContext = {};
      Ref<InspectorPanel> inspector = PanelManager::Get().GetPanel<InspectorPanel>("InspectorPanel");
      inspector->UpdateEntitySelection();
    }
  }
  ImGui::EndPopup();
}

} // namespace CatEngine
