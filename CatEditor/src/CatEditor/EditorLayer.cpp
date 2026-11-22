#include "EditorLayer.h"

#include "CatEngine/Core/Application.h"
#include "CatEngine/Core/Log.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
#include "CatEngine/Editor/Panels/InpectorPanel.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Prefab/Prefab.h"
#include "CatEngine/Prefab/PrefabSerializer.h"
#include "imgui.h"
#include <filesystem>
#include <imgui_internal.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ImGuizmo.h"

#include <glad/glad.h>

#include "CatEngine/Scripting/ScriptEngine.h"

#include "CatEngine/AssetManager/AssetManager.h"

#include "CatEngine/Editor/Panels/AssetBrowserPanel.h"
#include "CatEngine/Editor/Panels/SceneHierarchyPanel.h"

#include "CatEngine/Editor/IconManager.h"

namespace CatEngine {
void EditorLayer::OnAttach() {
  s_Instance = this;

  IconManager::Init();

  FramebufferSpecification fbSpec;
  fbSpec.Attachments = {FramebufferTextureFormat::RGBA8,
                        FramebufferTextureFormat::RED_INTEGER,
                        FramebufferTextureFormat::Depth};
  fbSpec.Width = 1280;
  fbSpec.Height = 720;
  m_SceneViewportPanel = SceneViewportPanel(fbSpec);
  m_SceneCameraPanel = SceneViewportPanel(fbSpec);

  m_EditorScene = CreateRef<Scene>();

  m_MouseInUse = false;

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);
  m_CurrentScene = m_EditorScene;
  m_PanelManager = CreateScope<PanelManager>();

  m_PanelManager->AddPanel<SceneHierarchyPanel>("SceneHierarchyPanel", true);
  m_PanelManager->AddPanel<InspectorPanel>("InspectorPanel", true);
  m_PanelManager->AddPanel<AssetBrowserPanel>("AssetBrowserPanel", true);
  m_PanelManager->AddPanel<ConsolePanel>("ConsolePanel", true);

  auto commandLineArgs = Application::Get().GetSpecification().CommandlineArgs;
  if (commandLineArgs.Count > 1) {
    auto projectFilePath = commandLineArgs[1];
    OpenProject(projectFilePath);
    m_ProjectActive = true;
  }

  m_EditorCamera = EditorCamera(30.f, 1.778f, 0.1f, 1000.f);
}

void EditorLayer::OnDetach() { s_Instance = nullptr; }

void EditorLayer::OnUpdate(Time deltaTime) {
  CE_PROFILE_FUNCTION();
  m_DeltaTime = deltaTime;

  m_BlockMouseEvents = m_SceneViewportPanel.IsHovered();
  m_BlockKeyboardEvents = m_SceneViewportPanel.IsFocused();

  if (m_ProjectActive) {
    Project::GetActive()->UpdateAssetManager();
    OnUpdateEditor(deltaTime);
    switch (m_SceneState) {
      case SceneState::Edit: {
        OnUpdateMainCameraPreview(deltaTime);
        ScriptEngine::GetMutable().PollJIT();
        break;
      }
      case SceneState::Play: {
        OnUpdateRuntime(deltaTime);
        break;
      }
      case SceneState::Simulate: {
        OnUpdateSimulation(deltaTime);
        break;
      }
      case SceneState::Pause: {
        OnUpdateRuntime(deltaTime);
        break;
      }
    }
  } else {
    OpenProject();
  }
}

void EditorLayer::OnImGuiDraw() {

  CE_PROFILE_FUNCTION();

  static bool dockspaceOpen = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("MyDockSpace", &dockspaceOpen, window_flags);
  ImGui::PopStyleVar(3);
  {
    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    io.FontGlobalScale = 1.25f;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

    UI_MenuBar();
    UI_Viewport();

    m_PanelManager->OnImGuiRender();

    UI_Toolbar();

    ImGui::PopStyleVar();
  }
  ImGui::End();
}

void EditorLayer::OnOverlayRender() {
  Renderer2D::BeginScene(m_EditorCamera);
  {
    Entity selectedEntity =
        m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")
            ->GetSelectedEntity();

    if (selectedEntity) {
      auto &tc = selectedEntity.GetComponent<TransformComponent>();
      glm::vec3 position = tc.WorldPosition;
      glm::mat4 rotation = glm::toMat4(glm::quat(tc.WorldRotation));
      glm::vec3 scale = tc.WorldScale;
      glm::mat4 transform = glm::translate(glm::mat4(1.f), position) *
                            rotation * glm::scale(glm::mat4(1.f), scale);
      Renderer2D::DrawRect(transform, glm::vec4(0.85f, 0.1f, 0.85f, 1.0f));
    }
  }

  {
    auto view =
        m_EditorScene->GetAllComponentsWith<TransformComponent,
                                            CircleCollider2DComponent>();

    for (auto entity : view) {
      auto [tc, cc2D] =
          view.get<TransformComponent, CircleCollider2DComponent>(entity);
      if (cc2D.ShowColliderBounds) {
        glm::vec3 position = tc.WorldPosition + glm::vec3(cc2D.Offset, 0.0001f);
        glm::vec3 scale = tc.WorldScale * glm::vec3(cc2D.Radius * 2.f);

        CircleRendererComponent crc;
        crc.Color = glm::vec4(0.2f, 0.2f, 0.95f, 1.0f);
        crc.Thickness = 0.025f;
        crc.Fade = 0.0001f;

        glm::mat4 transform = glm::translate(glm::mat4(1.f), position) *
                              glm::scale(glm::mat4(1.f), scale);
        Renderer2D::DrawCircle(transform, crc, -1);
      }
    }
  }

  {
    auto view = m_EditorScene->GetAllComponentsWith<TransformComponent,
                                                    BoxCollider2DComponent>();

    for (auto entity : view) {
      auto [tc, bc2D] =
          view.get<TransformComponent, BoxCollider2DComponent>(entity);
      if (bc2D.ShowColliderBounds) {
        glm::vec3 scale = tc.WorldScale * glm::vec3(bc2D.Size * 2.f, 1.f);

        // Thing to note - You have to translate & rotate Collider2D's after the
        // objecct itself
        glm::mat4 transform =
            glm::translate(glm::mat4(1.f), tc.WorldPosition) *
            glm::rotate(glm::mat4(1.0f), tc.WorldRotation.z,
                        glm::vec3(0, 0, 1)) *
            glm::translate(glm::mat4(1.0f), glm::vec3(bc2D.Offset, 0.001f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(bc2D.Rotation),
                        glm::vec3(0, 0, 1)) *
            glm::scale(glm::mat4(1.f), scale);

        Renderer2D::DrawRect(transform, glm::vec4(0.2f, 0.2f, 0.95f, 1.0f));
      }
    }
  }
  Renderer2D::EndScene();
}

void EditorLayer::ImGuizmoDraw(Entity selectedEntity,
                               const glm::mat4 &cameraProjection,
                               glm::mat4 cameraView) {
  // Entity transform
  auto &tc = selectedEntity.GetComponent<TransformComponent>();
  glm::mat4 transform = selectedEntity.GetWorldTransform();

  // Entity Children
  auto &rc = selectedEntity.GetComponent<RelationshipComponent>();

  // Snapping
  bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
  float snapValue = 0.5f; // Snap to 0.5m for translation/scale
  // Snap to 45 degrees for rotation
  if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
    snapValue = 45.0f;

  float snapValues[3] = {snapValue, snapValue, snapValue};

  ImGuizmo::Manipulate(
      glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
      (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL,
      glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

  if (ImGuizmo::IsUsing()) {
    // 1. Get the parent's world matrix (if a parent exists)
    glm::mat4 parentWorldMatrix = glm::mat4(1.0f);
    if (rc.Parent != 0) {
      Entity parent = m_CurrentScene->GetEntityByUUID(rc.Parent);
      if (parent) {
        parentWorldMatrix = parent.GetWorldTransform();
      }
    }

    // 2. Convert the manipulated World Matrix back into Local Space
    glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * transform;

    // 3. Decompose the local matrix
    glm::vec3 localPosition, localRotation, localScale;
    Math::DecomposeTransform(localMatrix, localPosition, localRotation,
                             localScale);

    // 4. Update your local component fields safely using your delta rotation
    // trick
    glm::vec3 deltaRotation = localRotation - tc.LocalRotation;
    tc.LocalPosition = localPosition;
    tc.LocalRotation += deltaRotation;
    tc.LocalScale = localScale;
  }
}

void EditorLayer::OnUpdateEditor(Time deltaTime) {
  if (m_SceneViewportPanel.IsActive()) {
    if (m_SceneViewportPanel.FramebufferResized()) {
      m_EditorCamera.SetViewportSize((float)m_SceneViewportPanel.GetWidth(),
                                     (float)m_SceneViewportPanel.GetHeight());
    }

    m_EditorCamera.AllowEvents(!ImGuizmo::IsUsing() && m_BlockMouseEvents);

    m_EditorCamera.OnUpdate(deltaTime);

    m_SceneViewportPanel.BindFramebuffer();

    if (!m_LoadedPrefab)
      RenderCommand::Clear({0.1, 0.1, 0.12, 1.0});
    else
      RenderCommand::Clear({0.1, 0.17, 0.3, 1.0});

    m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

    m_CurrentScene->OnUpdateEditor(deltaTime, m_EditorCamera);

    auto [mx, my] = ImGui::GetMousePos();
    const glm::vec2 *viewportBounds = m_SceneViewportPanel.GetPanelBounds();
    mx -= viewportBounds[0].x;
    my -= viewportBounds[0].y;
    glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];
    my = viewportSize.y - my;

    int mouseX = (int)mx;
    int mouseY = (int)my;

    if ((mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x &&
         mouseY < (int)viewportSize.y)) {
      int pixelData = m_SceneViewportPanel.ReadPixelData(1, mouseX, mouseY);
      m_HoveredEntity = pixelData == -1 ? Entity()
                                        : Entity((entt::entity)pixelData,
                                                 m_CurrentScene.get());
    }

    OnOverlayRender();

    m_SceneViewportPanel.UnbindFramebuffer();
  }
}

void EditorLayer::OnUpdateMainCameraPreview(Time deltaTime) {
  if (m_SceneCameraPanel.IsActive()) {
    m_SceneCameraPanel.FramebufferResized();

    m_SceneCameraPanel.BindFramebuffer();
    RenderCommand::Clear({0.1, 0.1, 0.1, 1.0});
    m_CurrentScene->OnUpdateMainCameraPreview(deltaTime);

    m_SceneCameraPanel.UnbindFramebuffer();
  }
}

void EditorLayer::OnUpdateRuntime(Time deltaTime) {
  if (m_SceneCameraPanel.IsActive()) {
    m_SceneCameraPanel.FramebufferResized();

    m_SceneCameraPanel.BindFramebuffer();
    RenderCommand::Clear({0.1, 0.1, 0.1, 1.0});
    m_CurrentScene->OnUpdateRuntime(deltaTime);

    m_SceneCameraPanel.UnbindFramebuffer();
  } else if (m_SceneViewportPanel.IsActive()) {
    m_SceneViewportPanel.BindFramebuffer();

    RenderCommand::Clear({0.1, 0.1, 0.1, 1.0});

    m_CurrentScene->OnUpdateRuntime(deltaTime);

    m_SceneViewportPanel.UnbindFramebuffer();
  }
}

void EditorLayer::OnUpdateSimulation(Time deltaTime) {
  if (m_SceneViewportPanel.IsActive()) {
    if (m_SceneViewportPanel.FramebufferResized()) {
      m_EditorCamera.SetViewportSize((float)m_SceneViewportPanel.GetWidth(),
                                     (float)m_SceneViewportPanel.GetHeight());
    }

    m_EditorCamera.AllowEvents(!ImGuizmo::IsUsing() && m_BlockMouseEvents);

    m_EditorCamera.OnUpdate(deltaTime);

    m_SceneViewportPanel.BindFramebuffer();

    RenderCommand::Clear({0.1, 0.1, 0.1, 1.0});
    m_SceneViewportPanel.ClearFramebufferAttachment(1, -1);

    m_SimulationScene->OnUpdateSimulation(deltaTime, m_EditorCamera);

    auto [mx, my] = ImGui::GetMousePos();
    const glm::vec2 *viewportBounds = m_SceneViewportPanel.GetPanelBounds();
    mx -= viewportBounds[0].x;
    my -= viewportBounds[0].y;
    glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];
    my = viewportSize.y - my;

    int mouseX = (int)mx;
    int mouseY = (int)my;

    if ((mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x &&
         mouseY < (int)viewportSize.y)) {
      int pixelData = m_SceneViewportPanel.ReadPixelData(1, mouseX, mouseY);
      m_HoveredEntity = pixelData == -1 ? Entity()
                                        : Entity((entt::entity)pixelData,
                                                 m_CurrentScene.get());
    }
    m_SceneViewportPanel.UnbindFramebuffer();

    if (m_SceneCameraPanel.IsActive()) {
      m_SceneCameraPanel.BindFramebuffer();

      RenderCommand::Clear({0.1, 0.1, 0.1, 1.0});
      m_SceneCameraPanel.ClearFramebufferAttachment(1, -1);
      m_SimulationScene->OnUpdateMainCameraPreview(deltaTime);
      m_SceneCameraPanel.UnbindFramebuffer();
    }
  }
}

void EditorLayer::UI_Viewport() {
  if (m_SceneViewportPanel.IsActive()) {
    ImGui::Begin("Viewport");
    m_SceneViewportPanel.OnImGuiRender();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
        const char* raw_str = static_cast<const char*>(payload->Data);
        size_t len = static_cast<size_t>(payload->DataSize);
        if (len > 0 && raw_str[len - 1] == '\0') {
          --len;
        }
        std::string_view sv(raw_str, len);
        unsigned long value = 0;

        bool isAsset = false;
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec == std::errc())
          isAsset = true;
        if (isAsset) {
          AssetHandle handle = value;
          const AssetType type = AssetManager::GetAssetType(handle);

          switch (type) {
            case AssetType::Scene: {
              OpenScene(handle);
              break;
            }
            case AssetType::Texture2D: {
              if (m_HoveredEntity &&
                m_HoveredEntity.HasComponent<SpriteRendererComponent>())
                m_HoveredEntity.GetComponent<SpriteRendererComponent>().Texture =
                  handle;
              break;
            }
            case AssetType::Prefab: {
              m_CurrentScene->InstantiatePrefab(handle);
              break;
            }
            default:
              break;
          }
        }
      }
      ImGui::EndDragDropTarget();
    }
    UI_Gizmos();
    ImGui::End();
  }

  if (m_SceneCameraPanel.IsActive()) {
    ImGui::Begin("Main Camera (preview)");
    m_SceneCameraPanel.OnImGuiRender();
    ImGui::End();
  }
}

void EditorLayer::UI_Gizmos() {
  Entity selectedEntity =
      m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")
          ->GetSelectedEntity();
  if (selectedEntity && m_GizmoType != -1) {
    auto &stateComponent = selectedEntity.GetComponent<ObjectStateComponent>();

    if (stateComponent.isLocked)
      return;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    const glm::vec2 *viewportBounds = m_SceneViewportPanel.GetPanelBounds();

    ImGuizmo::SetRect(viewportBounds[0].x, viewportBounds[0].y,
                      viewportBounds[1].x - viewportBounds[0].x,
                      viewportBounds[1].y - viewportBounds[0].y);

    // Camera

    {
      const glm::mat4 &cameraProjection = m_EditorCamera.GetProjection();
      glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
      ImGuizmoDraw(selectedEntity, cameraProjection, cameraView);
    }
  }
}

void EditorLayer::UI_Toolbar() {
  if (m_LoadedPrefab)
    return;

  ImGuiWindowClass tabWindowClass;
  tabWindowClass.DockNodeFlagsOverrideSet =
      ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResizeFlagsMask_;
  ImGui::SetNextWindowClass(&tabWindowClass);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 2});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, {0, 0});
  ImGuiWindowFlags tabStyle = ImGuiWindowFlags_NoDecoration |
                              ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse;

  ImGui::Begin("##toolbar", nullptr, tabStyle);
  {

    float size = ImGui::GetWindowHeight() - 4.f;
    Ref<Texture2D> runtimeIcon = m_SceneState == SceneState::Play
                                     ? IconManager::GetIcon("Stop-Runtime")
                                     : IconManager::GetIcon("Start-Runtime");
    Ref<Texture2D> pauseIcon;
    if (m_SceneState == SceneState::Edit)
      pauseIcon = m_IsScenePaused
                      ? IconManager::GetIcon("Pause-Runtime-Selected")
                      : IconManager::GetIcon("Pause-Runtime");
    else if (m_SceneState == SceneState::Play)
      pauseIcon = m_IsScenePaused ? IconManager::GetIcon("NextFrame-Runtime")
                                  : IconManager::GetIcon("Pause-Runtime");
    else if (m_SceneState == SceneState::Simulate)
      pauseIcon = m_IsScenePaused
                      ? IconManager::GetIcon("Pause-Runtime-Selected")
                      : IconManager::GetIcon(
                            "Pause-Runtime"); // TODO: Disable the play & pause
                                              // buttons during simulation

    Ref<Texture2D> simulationIcon =
        m_SceneState == SceneState::Simulate
            ? IconManager::GetIcon("Stop-Runtime")
            : IconManager::GetIcon("Start-Simulation");

    ImGui::SetCursorPosX(
        (ImGui::GetWindowContentRegionMax().x * 0.5f - (size * 0.5f)));
    if (ImGui::ImageButton("##PLAY",
                           (ImTextureID)(uint64_t)runtimeIcon->GetRendererID(),
                           ImVec2(size * 1.403f, size), {0, 0}, {1, 1})) {
      if (m_SceneState == SceneState::Edit)
        OnScenePlay();
      else if (m_SceneState == SceneState::Play)
        OnSceneStop();
    }
    ImGui::SameLine();
    if (ImGui::ImageButton("##PAUSE",
                           (ImTextureID)(uint64_t)pauseIcon->GetRendererID(),
                           ImVec2(size * 1.403f, size), {1, 0}, {0, 1})) {
      if (m_SceneState == SceneState::Edit) {
        m_IsScenePaused = !m_IsScenePaused;
        OnScenePause(m_IsScenePaused);
      } else if (m_SceneState == SceneState::Play) {
        m_IsScenePaused = !m_IsScenePaused;
        OnScenePause(m_IsScenePaused);
      }
    }
    ImGui::SameLine();
    if (ImGui::ImageButton(
            "##SIMULATE",
            (ImTextureID)(uint64_t)simulationIcon->GetRendererID(),
            ImVec2(size * 1.403f, size), {0, 0}, {1, 1})) {
      if (m_SceneState == SceneState::Edit)
        OnSceneSimulateStart();
      else if (m_SceneState == SceneState::Simulate)
        OnSceneSimulateStop();
    }

    ImGui::PopStyleVar(2);
  }
  ImGui::End();
}

void EditorLayer::UI_MenuBar() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        SaveScene();
        SaveProject();
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scene")) {
      const char *runtimeText =
          m_SceneState == SceneState::Edit ? "Start Runtime" : "Stop Runtime";
      if (ImGui::MenuItem(runtimeText, "Ctrl+F5"))
        m_SceneState == SceneState::Edit ? OnScenePlay() : OnSceneStop();

      if (ImGui::MenuItem("Simulate Runtime", "Ctrl+F7"))
        m_SceneState == SceneState::Edit ? OnSceneSimulateStart()
                                         : OnSceneSimulateStop();

      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

void EditorLayer::SaveSceneAs() {
  CE_PROFILE_FUNCTION();

  if (m_SceneState != SceneState::Edit)
    return;

  m_EditorScenePath = FileDialog::SaveFile({{"CatEngine Scene", "catscene"}});
  if (!m_EditorScenePath.empty())
    SaveScene();
}

void EditorLayer::SaveScene() {
  CE_PROFILE_FUNCTION();

  if (m_SceneState != SceneState::Edit)
    return;

  auto sceneMetaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(m_CurrentScene->GetAssetHandle());
  auto absPath = std::filesystem::absolute(Project::GetAssetDirectory() / sceneMetaData.SourceFilePath);
  CE_API_WARN("Attempting to save at {}", absPath.generic_string());

  if (m_LoadedPrefab) {
    m_LoadedPrefab->SetScene(m_CurrentScene);
    PrefabSerializer serializer(m_LoadedPrefab);
    serializer.Serialize(absPath, {});
  } else {
    SceneSerializer serializer(m_CurrentScene);
    serializer.Serialize(absPath);
    if (!AssetManager::IsAssetHandleValid(m_CurrentSceneHandle)) {
      CE_API_WARN("Failed to find valid asset handle for scene {}", absPath.generic_string());
    }
  }
}

void EditorLayer::NewSampleScene(const std::filesystem::path &loadPath) {
  CE_PROFILE_FUNCTION();

  std::filesystem::path sampleScenePath = std::filesystem::absolute(Application::Get().GetMainPath() / "Resources/Samples/SampleScene2D.catscene");
  std::filesystem::path newSourcePath = loadPath / "SampleScene2D.catscene";
  int i = 1;
  while (std::filesystem::exists(newSourcePath)) {
    std::string fileName = newSourcePath.filename();
    std::string extension = newSourcePath.extension();
    newSourcePath = newSourcePath.parent_path();

    fileName = fileName.erase(fileName.find('.'));
    newSourcePath = newSourcePath / (fileName + std::to_string(i) + extension);
  }

  std::filesystem::copy(sampleScenePath, newSourcePath);

  auto sceneHandle = Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(newSourcePath);


  Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(sceneHandle);
  Ref<Scene> newScene = CreateRef<Scene>();
  readOnlyScene->CopyTo(newScene);
  Project::GetActive()->OpenScene(sceneHandle);

  m_EditorScene = newScene;

  m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(),
                                  m_SceneViewportPanel.GetHeight());

  m_PanelManager->SetSceneContext(m_EditorScene);

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);

  m_CurrentScene = m_EditorScene;
}

void EditorLayer::NewEmptyScene(const std::filesystem::path &loadPath) {
  CE_PROFILE_FUNCTION();

  std::filesystem::path sampleScenePath = std::filesystem::absolute(Application::Get().GetMainPath() / "Resources/Samples/EmptyScene2D.catscene");
  std::filesystem::path newSourcePath = loadPath / "EmptyScene2D.catscene";
  int i = 1;
  while (std::filesystem::exists(newSourcePath)) {
    std::string fileName = newSourcePath.filename();
    std::string extension = newSourcePath.extension();
    newSourcePath = newSourcePath.parent_path();

    fileName = fileName.erase(fileName.find('.'));
    newSourcePath = newSourcePath / (fileName + std::to_string(i) + extension);
  }

  std::filesystem::copy(sampleScenePath, newSourcePath);

  auto sceneHandle = Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(newSourcePath);


  Ref<Scene> readOnlyScene = AssetManager::GetAsset<Scene>(sceneHandle);
  Ref<Scene> newScene = CreateRef<Scene>();
  readOnlyScene->CopyTo(newScene);
  Project::GetActive()->OpenScene(sceneHandle);

  m_EditorScene = newScene;

  m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(),
                                  m_SceneViewportPanel.GetHeight());

  m_PanelManager->SetSceneContext(m_EditorScene);

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);

  m_CurrentScene = m_EditorScene;
}

void EditorLayer::OpenScene(const AssetHandle &handle) {
  CE_PROFILE_FUNCTION();

  if (!AssetManager::IsAssetHandleValid(handle)) {
    CE_API_ERROR("Scene Asset Handle is Invalid!");
    auto console = m_PanelManager->GetPanel<ConsolePanel>("ConsolePanel");
    console->WriteError("Scene Asset Handle: " + std::to_string(handle) + " is invalid!");
    return;
  }

  Ref<Scene> readOnlyScene = nullptr;
  Ref<Asset> asset = Project::GetActive()->GetEditorAssetManager()->GetAsset(handle);
  switch (asset->GetType()) {
    case AssetType::Scene: {
      readOnlyScene = (Ref<Scene> &)asset;
      m_LoadedPrefab = nullptr;
      m_SceneCameraPanel.SetActive(true);
      break;
    }
    case AssetType::Prefab: {
      Ref<Prefab> prefab = (Ref<Prefab> &)asset;
      readOnlyScene = prefab->GetScene();
      m_LoadedPrefab = prefab;
      m_SceneCameraPanel.SetActive(false);
      break;
    }
    default:
      return;
  }

  Ref<Scene> newScene = CreateRef<Scene>();
  readOnlyScene->CopyTo(newScene);
  Project::GetActive()->OpenScene(handle);

  m_EditorScene = newScene;

  m_EditorScene->OnViewportResize(m_SceneViewportPanel.GetWidth(),
                                  m_SceneViewportPanel.GetHeight());

  m_PanelManager->SetSceneContext(m_EditorScene);

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);

  m_CurrentScene = m_EditorScene;

  std::filesystem::path savePath =
      Project::GetAssetDirectory() /
      Project::GetActive()->GetEditorAssetManager()->GetSourceFilePath(handle);

  m_EditorScenePath = savePath;
  m_CurrentSceneHandle = handle;
}


void EditorLayer::OpenPreviousScene() {
  auto &scenes = Project::GetOpenScenes();
  if (scenes.empty()) {
    NewEmptyScene(Project::GetAssetDirectory());
    return;
  }

  auto &handle = scenes.at(scenes.size() - 1);
  OpenScene(handle);
}

void EditorLayer::SaveProject() {
  Project::Save(m_CurrentProjectPath);
}

void EditorLayer::OpenProject() {
  std::filesystem::path filePath =
      FileDialog::OpenFile({{"CatEngine Project", "catproj"}});
  if (!filePath.empty()) {
    OpenProject(filePath);
  } else {
    NewProject();
  }
}

void EditorLayer::OpenProject(const std::filesystem::path &filePath) {
  if (filePath.extension().string() != ".catproj") {
    CE_CLI_WARN("Could not load {0} - Not a project file!",
                filePath.filename().string());
    return;
  }
  if (Project::Load(filePath)) {
    m_CurrentProjectPath = filePath;
    m_ProjectActive = true;
    Ref<AssetBrowserPanel> assetBrowser = m_PanelManager->GetPanel<AssetBrowserPanel>("AssetBrowserPanel");
    assetBrowser->SetCurrentDirectory(Project::GetConfig().LastLoadedAssetPath);
    auto &scenes = Project::GetConfig().OpenScenes;
    // We store scenes from most oldest to newest
    if (!scenes.empty()) {
      auto startScenePath = scenes.at(scenes.size() - 1);
      if (AssetManager::IsAssetHandleValid(startScenePath))
        OpenScene(startScenePath);
    } else {
      NewSampleScene(Project::GetAssetDirectory());
    }
    ScriptEngine::GetMutable().Init();
  }
}

void EditorLayer::NewProject() { Project::New(); }

void EditorLayer::OnScenePlay() {
  CE_PROFILE_FUNCTION();

  m_SceneState = SceneState::Play;

  m_RuntimeScene = CreateRef<Scene>();

  m_EditorScene->CopyTo(m_RuntimeScene);

  m_PanelManager->SetSceneContext(m_RuntimeScene);

  m_SceneViewportPanel.SetContext(m_RuntimeScene);
  m_SceneCameraPanel.SetContext(m_RuntimeScene);
  ScriptEngine::GetMutable().SetSceneContext(m_RuntimeScene);

  m_RuntimeScene->OnRuntimeStart();

  if (m_IsScenePaused)
    m_RuntimeScene->OnPauseStart();

  m_CurrentScene = m_RuntimeScene;
}

void EditorLayer::OnScenePause(bool isPaused) {

  if (m_SceneState == SceneState::Edit)
    return;

  if (isPaused)
    m_CurrentScene->OnPauseStart();
  else
    m_CurrentScene->OnPauseStop();
}

void EditorLayer::OnSceneStop() {
  CE_PROFILE_FUNCTION();

  m_SceneState = SceneState::Edit;
  m_RuntimeScene->OnRuntimeStop();
  m_RuntimeScene = nullptr;

  m_PanelManager->SetSceneContext(m_EditorScene);

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);

  m_CurrentScene = m_EditorScene;
}

void EditorLayer::OnSceneSimulateStart() {
  CE_PROFILE_FUNCTION();

  m_SceneState = SceneState::Simulate;

  m_SimulationScene = CreateRef<Scene>();

  m_EditorScene->CopyTo(m_SimulationScene);

  m_PanelManager->SetSceneContext(m_SimulationScene);

  m_SceneViewportPanel.SetContext(m_SimulationScene);
  m_SceneCameraPanel.SetContext(m_SimulationScene);
  ScriptEngine::GetMutable().SetSceneContext(m_SimulationScene);
  m_SimulationScene->OnSimulationStart();
  m_CurrentScene = m_SimulationScene;
}

void EditorLayer::OnSceneSimulateStop() {
  CE_PROFILE_FUNCTION();

  m_SceneState = SceneState::Edit;
  m_SimulationScene->OnSimulationStop();

  m_SimulationScene = nullptr;

  m_PanelManager->SetSceneContext(m_EditorScene);

  m_SceneViewportPanel.SetContext(m_EditorScene);
  m_SceneCameraPanel.SetContext(m_EditorScene);
  ScriptEngine::GetMutable().SetSceneContext(m_EditorScene);
  m_CurrentScene = m_EditorScene;
}

void EditorLayer::DuplicateEntity() {
  CE_PROFILE_FUNCTION();

  Entity entity = {};
  Entity selectedEntity =
      m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")
          ->GetSelectedEntity();
  if (entity) {
    m_CurrentScene->DuplicateEntity(entity);
  }
}
void EditorLayer::DeleteEntity() {
  CE_PROFILE_FUNCTION();

  Entity entity = {};
  Entity selectedEntity =
      m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")
          ->GetSelectedEntity();
  if (entity) {
    m_CurrentScene->DeleteEntity(entity);
    // m_SceneHierarchyPanel.SetSelectedEntity();
  }
}
void EditorLayer::CopyEntity() {
  CE_PROFILE_FUNCTION();

  Entity entity = {};
  Entity selectedEntity =
      m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")
          ->GetSelectedEntity();
  if (entity) {
    m_CopiedEntity = entity;
  }
}
void EditorLayer::PasteEntity() {
  CE_PROFILE_FUNCTION();

  if (m_CopiedEntity) {
    // TODO : Make it to where entity doesn't need to exist
    Entity pastedEntity = m_CurrentScene->PasteEntity(m_CopiedEntity);
    // m_SceneHierarchyPanel.SetSelectedEntity(pastedEntity);
  }
}

void EditorLayer::OnEvent(Event &e) {
  EventDispatcher dispatcher(e);
  m_EditorCamera.OnEvent(e);
  dispatcher.Dispatch<WindowResizeEvent>(
      BIND_EVENT_FN(EditorLayer::OnWindowResize));
  dispatcher.Dispatch<KeyPressedEvent>(
      BIND_EVENT_FN(EditorLayer::OnKeyPressed));
  dispatcher.Dispatch<MouseButtonPressedEvent>(
      BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnWindowResize(WindowResizeEvent &e) {
  Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

  return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent &e) {

  bool control = Input::IsKeyPressed(KeyCode::LeftControl) ||
                 Input::IsKeyPressed(KeyCode::RightControl);
  bool shift = Input::IsKeyPressed(KeyCode::LeftShift) ||
               Input::IsKeyPressed(KeyCode::RightShift);
  // Shortcuts
  switch (e.GetKeyCode()) {
  case KeyCode::C: {
    if (control && shift) {
    } else if (control)
      CopyEntity();
    break;
  }
  case KeyCode::D: {
    if (control && shift) {
    } else if (control)
      DuplicateEntity();
    break;
  }
  case KeyCode::V: {
    if (control && shift) {
    } else if (control)
      PasteEntity();
    break;
  }
  case KeyCode::S: {
    if (control && shift) {
      SaveSceneAs();
      SaveProject();
    } else if (control) {
      SaveScene();
      SaveProject();
    }
    break;
  }
  case KeyCode::O: {
    if (control) {
      // OpenScene();
      SaveProject();
    }
    break;
  }
  case KeyCode::N: {
      if (control) {
        if (shift)
          NewEmptyScene(Project::GetAssetDirectory());
        else
          NewSampleScene(Project::GetAssetDirectory());
      }
    break;
  }
  case KeyCode::F5: {
    if (control)
      m_SceneState == SceneState::Edit ? OnScenePlay() : OnSceneStop();
    break;
  }
  case KeyCode::F7: {
    if (control)
      m_SceneState == SceneState::Edit ? OnSceneSimulateStart()
                                       : OnSceneSimulateStop();
    break;
  }
  case KeyCode::Q: {
    if (m_BlockKeyboardEvents) // if false, block events
      m_GizmoType = ImGuizmo::OPERATION::BOUNDS;
    break;
  }
  case KeyCode::W: {
    if (m_BlockKeyboardEvents)
      m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
    break;
  }
  case KeyCode::E: {
    if (m_BlockKeyboardEvents)
      m_GizmoType = ImGuizmo::OPERATION::ROTATE;
    break;
  }
  case KeyCode::R: {
    if (m_BlockKeyboardEvents)
      m_GizmoType = ImGuizmo::OPERATION::SCALE;
    break;
  }
  case KeyCode::Delete: {
    DeleteEntity();
  }
  default:
    break;
  }

  return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e) {
  bool alt = Input::IsKeyPressed(KeyCode::LeftAlt) ||
             Input::IsKeyPressed(KeyCode::RightAlt);
  if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft)) {
    if (m_SceneViewportPanel.IsHovered() && !ImGuizmo::IsOver() && !alt) {
      m_PanelManager->GetPanel<InspectorPanel>("InspectorPanel")
          ->SetEntitySelectionContext(m_HoveredEntity);
      m_PanelManager->GetPanel<SceneHierarchyPanel>("SceneHierarchyPanel")->SetSelectedEntity(m_HoveredEntity);
    }
  }

  return false;
}

} // namespace CatEngine
