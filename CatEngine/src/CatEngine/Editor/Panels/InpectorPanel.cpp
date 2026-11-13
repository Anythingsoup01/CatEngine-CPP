#include "InpectorPanel.h"
#include "CatEngine/Editor/ImGui/CEImGui.h"
#include "CatEngine/Editor/Panels/DrawComponents.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Scene.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace CatEngine {

InspectorPanel::InspectorPanel(const Ref<Scene> &context)
    : m_Context(context) {}

void InspectorPanel::OnImGuiRender(bool &isOpen) {
  ImGui::Begin("Inspector");
  if (m_EntitySelection || m_AssetSelection) {
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    const ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

    if (m_EntitySelection) {
      DrawComponents(m_EntitySelection);

      bool Components2DAvaliable =
          ((!m_EntitySelection.HasComponent<SpriteRendererComponent>() &&
            !m_EntitySelection.HasComponent<CircleRendererComponent>()) ||
           !m_EntitySelection.HasComponent<Rigidbody2DComponent>() ||
           (!m_EntitySelection.HasComponent<BoxCollider2DComponent>() &&
            !m_EntitySelection.HasComponent<CircleCollider2DComponent>()));

      float lineHeight =
          GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;

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

    } else if (m_AssetSelection) {
      DrawAsset(m_AssetSelection);
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         (((float)contentRegionAvailable.x -
                           ((float)contentRegionAvailable.x / 2.f)) *
                          .5f));

    ImGui::Dummy(ImVec2{ImGui::GetContentRegionAvail().x, 10});
  }
  ImGui::End();
}

void InspectorPanel::SetEntitySelectionContext(Entity e) {
  m_EntitySelection = e;
  m_AssetSelection = 0;
}

void InspectorPanel::SetAssetSelectionContext(AssetHandle handle) {
  m_EntitySelection = {};
  m_AssetSelection = handle;
}

void InspectorPanel::DrawComponents(Entity selection) {
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

void InspectorPanel::DrawAsset(const AssetHandle &handle) {
  const auto &asset =
      Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);

  switch (asset->GetType()) {
  case AssetType::Scene: {
    Ref<Scene> sceneAsset = (Ref<Scene> &)asset;

    if (!ImGui::BeginTable("##SceneData", 2))
      break;

    ImGui::TableSetupColumn("##INFOTAG", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("##INFODATA", ImGuiTableColumnFlags_WidthStretch);
    ImGui::EndTable();
    break;
  }

  case AssetType::Texture2D: {
    static std::vector<const char *> s_TextureFormats = {
        "R8",
        "RGB8",
        "RGBA8",
    };

    static std::vector<const char *> s_TextureFilters = {"Linear", "Nearest"};

    static std::vector<const char *> s_TextureWrapFilters = {
        "Repeat",
        "Mirrored Repeat",
        "Clamp To Edge",
        "Clamp To Border",
    };
    Ref<Texture2D> textureAsset = (Ref<Texture2D> &)asset;

    const Asset::MetaData &metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(handle);

    ImGuiDraw::StaticString("Name", metaData.AssetName);
    ImGuiDraw::Combo("Min Filter", (int &)metaData.TextureMinFilter,
                     s_TextureFilters);
    ImGuiDraw::Combo("Mag Filter", (int &)metaData.TextureMagFilter,
                     s_TextureFilters);
    ImGuiDraw::Combo("Wrap Options", (int &)metaData.TextureWrap,
                     s_TextureWrapFilters);
    ImGuiDraw::StaticString("Format",
                            ImageFormatToString(metaData.TextureFormat));

    if (ImGui::Button("Apply")) {
      TextureSpecification spec;
      spec.MinFilter = metaData.TextureMinFilter;
      spec.MagFilter = metaData.TextureMagFilter;
      spec.WrapOption = metaData.TextureWrap;
      spec.Format = metaData.TextureFormat;

      Buffer data = TextureHelper::BufferTexture(
          Project::GetAssetDirectory() / metaData.FilePath, (int &)spec.Width,
          (int &)spec.Height);
      textureAsset->RecreateTextureWithNewSpecification(spec, data);
    }
  }
  }
}

template <typename T>
void InspectorPanel::DisplayAddComponentEntry(const std::string &entryName) {
  if (!m_EntitySelection.HasComponent<T>()) {
    if (ImGui::MenuItem(entryName.c_str())) {
      m_EntitySelection.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
}

template <typename T, typename O>
void InspectorPanel::DisplayAddComponentEntries(
    const std::string &entryOneName, const std::string &entryTwoName) {
    if (!m_EntitySelection.HasComponent<T>() &&
      !m_EntitySelection.HasComponent<O>()) {
    if (ImGui::MenuItem(entryOneName.c_str())) {
      m_EntitySelection.AddComponent<T>();
      ImGui::CloseCurrentPopup();
    }
  }
  if (!m_EntitySelection.HasComponent<O>() &&
      !m_EntitySelection.HasComponent<T>()) {
    if (ImGui::MenuItem(entryTwoName.c_str())) {
      m_EntitySelection.AddComponent<O>();
      ImGui::CloseCurrentPopup();
    }
  }
}

void InspectorPanel::DisplayAddScriptComponent(const std::string &name) {
  if (ImGui::MenuItem(name.c_str())) {
    m_EntitySelection.AddScriptComponent();
    ImGui::CloseCurrentPopup();
  }
}

} // namespace CatEngine
