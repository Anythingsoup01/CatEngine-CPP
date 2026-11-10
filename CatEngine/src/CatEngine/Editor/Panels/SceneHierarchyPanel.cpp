#include "SceneHierarchyPanel.h"
#include <cepch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CatEngine/Core/UUID.h"
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
    ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX |
                                 ImGuiTableFlags_BordersInnerV |
                                 ImGuiTableFlags_ScrollY;

    // Name, Visibility, Lock
    const int numberOfCols = 3;
    if (ImGui::BeginTable("##SceneHierarchy-Table", numberOfCols, tableFlags,
                          ImVec2(ImGui::GetContentRegionAvail()))) {

      ImGui::TableSetupColumn("Label");
      ImGui::TableSetupColumn("Visibility", ImGuiTableColumnFlags_WidthFixed,
                              25.0f);
      ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed, 25.0f);

      // Headers
      {
        ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
        for (int column = 0; column < ImGui::TableGetColumnCount(); column++) {
          ImGui::TableSetColumnIndex(column);
          const char *column_name = ImGui::TableGetColumnName(column);
          ImGui::TableHeader(column_name);
        }
      }

      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);

      {
        for (auto entity : m_Context->GetRootEntities()) {
          Entity e = m_Context->GetEntityByUUID(entity);
          auto &rel = e.GetComponent<RelationshipComponent>();
          if (rel.Parent == 0) { // Root Node
            DrawEntityNode(e);
          }
        }
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Spacing();
      ImGui::Dummy(ImGui::GetContentRegionAvail());

      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
          UUID droppedEntityID = *(UUID *)payload->Data;
          Entity droppedEntity = m_Context->GetEntityByUUID(droppedEntityID);

          if (droppedEntity) {
            // Dropped into empty space = Make it a root-level entity (Parent = 0)
            m_Context->ReparentEntity(droppedEntity, {}); 
          }
        }
        ImGui::EndDragDropTarget();
      }

      if (ImGui::BeginPopupContextWindow(nullptr,
                                         ImGuiPopupFlags_MouseButtonRight |
                                             ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Create")) {
          if (ImGui::MenuItem("Create Empty GameObject")) {
            const auto &entity = m_Context->CreateEntity("GameObject");
            m_SelectionContext = entity;
            ImGui::CloseCurrentPopup();
          }
          if (ImGui::MenuItem("Create Camera")) {
            const auto &entity = m_Context->CreateEntity("Camera");
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
  if (m_SelectionContext) {
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    DrawComponents(m_SelectionContext);

    float lineHeight =
        GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         (((float)contentRegionAvailable.x -
                           ((float)contentRegionAvailable.x / 2.f)) *
                          .5f));

    bool Components2DAvaliable =
        ((!m_SelectionContext.HasComponent<SpriteRendererComponent>() &&
          !m_SelectionContext.HasComponent<CircleRendererComponent>()) ||
         !m_SelectionContext.HasComponent<Rigidbody2DComponent>() ||
         (!m_SelectionContext.HasComponent<BoxCollider2DComponent>() &&
          !m_SelectionContext.HasComponent<CircleCollider2DComponent>()));

    if (ImGui::Button("Add Component",
                      ImVec2{contentRegionAvailable.x / 2.f, lineHeight}))
      ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent")) {
      DisplayAddComponentEntry<CameraComponent>("Camera Component");
      if (Components2DAvaliable) {
        bool opened = ImGui::TreeNodeEx("2D Components", treeNodeFlags);

        if (opened) {
          DisplayAddComponentEntries<SpriteRendererComponent,
                                     CircleRendererComponent>(
              "Sprite Renderer Component", "Circle Renderer Component");
          DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
          DisplayAddComponentEntries<BoxCollider2DComponent,
                                     CircleCollider2DComponent>(
              "Box Collider 2D", "Circle Collider 2D");
          ImGui::TreePop();
        }
      }
      DisplayAddScriptComponent("Script");
      ImGui::EndPopup();
    }
      ImGui::Dummy(ImVec2{ImGui::GetContentRegionAvail().x, 10});

  }
  ImGui::End();
}
void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
  ImGui::PushID(std::to_string(entity.GetUUID()).c_str());
  const char *name = "Unnamed Entity";
  name = entity.GetComponent<NameComponent>().Name.c_str();

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
    ImGui::Text("Moving: %s", name);
    ImGui::EndDragDropSource();
  }

 if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCENE_HIERARCHY_ITEM")) {
      UUID droppedEntityID = *(UUID *)payload->Data;
      Entity droppedEntity = m_Context->GetEntityByUUID(droppedEntityID);

      if (droppedEntity && droppedEntity != entity) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 itemRectMin = ImGui::GetItemRectMin();
        ImVec2 itemRectMax = ImGui::GetItemRectMax();
        float itemHeight = itemRectMax.y - itemRectMin.y;

        float relativeY = mousePos.y - itemRectMin.y;

        // Wased thresholds: Top 35% = Before, Bottom 35% = After, Middle 30% = Reparent
        if (relativeY < itemHeight * 0.35f) {
          // Drop Above (Sibling Reorder - Before)
          m_Context->MoveEntityBefore(droppedEntity, entity);
        } 
        else if (relativeY > itemHeight * 0.65f) {
          // Drop Below (Sibling Reorder - After)
          m_Context->MoveEntityAfter(droppedEntity, entity);
        } 
        else {
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

    auto* drawList = ImGui::GetWindowDrawList();
    if (relativeY < itemHeight * 0.35f) {
      // Draw line above item
      drawList->AddLine(itemRectMin, ImVec2(itemRectMax.x, itemRectMin.y), IM_COL32(0, 120, 255, 255), 2.0f);
    } else if (relativeY > itemHeight * 0.65f) {
      // Draw line below item
      drawList->AddLine(ImVec2(itemRectMin.x, itemRectMax.y), itemRectMax, IM_COL32(0, 120, 255, 255), 2.0f);
    }

    ImGui::EndDragDropTarget();
  }


  if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered()) {
    m_SelectionContext = entity;
  }

  bool entityDeleted = false;
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete"))
      entityDeleted = true;

    ImGui::EndPopup();
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
    if (m_SelectionContext == entity)
      m_SelectionContext = {};
    m_Context->DeleteEntity(entity);
  }

  // Visibility column

  ImGui::TableSetColumnIndex(1);

  bool visible = true;
  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight();

    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

    ImGui::Checkbox("##Visible", &visible);
  }

  // Lock column

  ImGui::TableSetColumnIndex(2);

  bool lock = false;
  {
    float columnWidth = ImGui::GetColumnWidth();
    float iconSize = ImGui::GetFrameHeight(); // square icon

    float cursorX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(cursorX + (columnWidth - iconSize) * 0.5f);

    ImGui::Checkbox("##lock", &lock);
  }
  ImGui::PopID();
}

void SceneHierarchyPanel::DrawComponents(Entity selection) {
  if (ImGui::BeginTable("##TAGLAYER", 2)) {
    ImGui::TableSetupColumn("TAG", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("LAYER", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    auto &tag = selection.GetComponent<TagComponent>().Tag;

    ImGui::PushItemWidth(-1.0f);
    char tagBuffer[256];
    memset(tagBuffer, 0, sizeof(tagBuffer));
    strncpy(tagBuffer, tag.c_str(), 256);
    ImGui::Text("Tag");
    ImGui::SameLine();
    if (ImGui::InputText("##T", tagBuffer, sizeof(tagBuffer))) {
      tag = std::string(tagBuffer);
    }
    ImGui::PopItemWidth();

    ImGui::TableSetColumnIndex(1);
    auto &layer = selection.GetComponent<LayerComponent>().Layer;

    ImGui::PushItemWidth(-1.0f);
    char layerBuffer[256];
    memset(layerBuffer, 0, sizeof(layerBuffer));
    strncpy(layerBuffer, layer.c_str(), 256);
    ImGui::Text("Layer");
    ImGui::SameLine();
    if (ImGui::InputText("##L", layerBuffer, sizeof(layerBuffer))) {
      layer = std::string(layerBuffer);
    }
    ImGui::PopItemWidth();

    ImGui::EndTable();
  }

  if (ImGui::BeginTable("##NAME", 1)) {
    ImGui::TableSetupColumn("NAME", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(-1.0f);
    auto &name = selection.GetComponent<NameComponent>().Name;

    char nameBuffer[256];
    memset(nameBuffer, 0, sizeof(nameBuffer));
    strncpy(nameBuffer, name.c_str(), 256);
    ImGui::Text("Name");
    ImGui::SameLine();
    if (ImGui::InputText("##N", nameBuffer, sizeof(nameBuffer))) {
      name = std::string(nameBuffer);
    }

    ImGui::PopItemWidth();

    ImGui::EndTable();
  }

  DrawComponentsHelper(selection);

  ImGui::Dummy(ImVec2{ImGui::GetContentRegionAvail().x, 50});
}

template <typename T>
void SceneHierarchyPanel::DisplayAddComponentEntry(
    const std::string &entryName) {
  if (!m_SelectionContext.HasComponent<T>()) {
    if (ImGui::MenuItem(entryName.c_str())) {
      m_SelectionContext.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
}
template <typename T, typename O>
void SceneHierarchyPanel::DisplayAddComponentEntries(
    const std::string &entryOneName, const std::string &entryTwoName) {
  if (!m_SelectionContext.HasComponent<T>() &&
      !m_SelectionContext.HasComponent<O>()) {
    if (ImGui::MenuItem(entryOneName.c_str())) {
      m_SelectionContext.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
  if (!m_SelectionContext.HasComponent<O>() &&
      !m_SelectionContext.HasComponent<T>()) {
    if (ImGui::MenuItem(entryTwoName.c_str())) {
      m_SelectionContext.AddComponent<O>();
      ImGui::CloseCurrentPopup();
    }
  }
}

void SceneHierarchyPanel::DisplayAddScriptComponent(const std::string &name) {
  if (ImGui::MenuItem(name.c_str())) {
    m_SelectionContext.AddScriptComponent();
    ImGui::CloseCurrentPopup();
  }
}

} // namespace CatEngine
