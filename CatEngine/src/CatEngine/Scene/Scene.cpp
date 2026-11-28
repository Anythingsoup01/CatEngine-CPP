#include "cepch.h" // IWYU pragma: keep
#include "Scene.h"

#include "Components/Components.h" // IWYU pragma: keep

#include "Entity.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Editor/PanelManager.h"
#include "CatEngine/Editor/Panels/ConsolePanel.h"
#include "CatEngine/Physics/Physics2D.h"
#include "CatEngine/Prefab/Prefab.h"
#include "CatEngine/Prefab/PrefabManager.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Renderer/Renderer2D.h"
#include "CatEngine/Renderer/RenderCommand.h"
#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine {

namespace SceneUtil {

static UIScreenRect ComputeNodeRect(const RectTransformComponent &rt, const UIScreenRect &parent, float scale) {
  const glm::vec2 parentSize = parent.Max - parent.Min;

  const glm::vec2 anchorMinPx = parent.Min + rt.AnchorMin * parentSize;
  const glm::vec2 anchorMaxPx = parent.Min + rt.AnchorMax * parentSize;
  const glm::vec2 anchorSize = anchorMaxPx - anchorMinPx;

  const glm::vec2 size = anchorSize + rt.Size * scale;

  // The pivot sits at the same normalized point between the anchors, plus the offset.
  const glm::vec2 pivotPos =
      anchorMinPx + anchorSize * rt.Pivot + rt.Position * scale;

  const glm::vec2 minCorner = pivotPos - size * rt.Pivot;
  return UIScreenRect{minCorner, minCorner + size};
}

static UIGroupState ResolveGroup(Entity e, const UIGroupState &parent) {
  if (!e.HasComponent<CanvasGroupComponent>())
    return parent;
  const auto &g = e.GetComponent<CanvasGroupComponent>();
  UIGroupState out;
  if (g.IgnoreParentGroups) {
    out = {g.Alpha, g.Interactable, g.BlocksRaycasts};
  } else {
    out.Alpha = parent.Alpha * g.Alpha;
    out.Interactable = parent.Interactable && g.Interactable;
    out.BlocksRaycasts = parent.BlocksRaycasts && g.BlocksRaycasts;
  }
  out.Alpha = std::clamp(out.Alpha, 0.0f, 1.0f);
  return out;
}

static uint32_t HandlerMask(Entity e) {
  uint32_t m = 0;
  if (e.HasComponent<SelectableComponent>())
    m |= UIMask::Hover | UIMask::Press | UIMask::Click;
  if (e.HasComponent<EventListenerComponent>())
    m |= e.GetComponent<EventListenerComponent>().Mask;
  m |= ScriptEngine::GetMutable().GetUIHandlerMask(e.GetUUID());
  return m;
}

static bool IsRaycastTarget(Entity e) {
  if (e.HasComponent<EventListenerComponent>()) return true; // invisible hit zone
  if (e.HasComponent<PanelComponent>() && e.GetComponent<PanelComponent>().RaycastTarget) return true;
  if (e.HasComponent<ButtonVisualComponent>() && e.GetComponent<ButtonVisualComponent>().RaycastTarget) return true;
  return ScriptEngine::GetMutable().GetUIHandlerMask(e.GetUUID()) != 0;
}

} // namespace SceneUtil

Scene::Scene() {}

Scene::~Scene() { ShutdownRuntime(); }

namespace SceneUtil {
template <typename... Component>
static void CopyComponentIfExists(Entity dst, Entity src, bool forced) {
  (
    [&]() {
      if (src.HasComponent<Component>()) {
        auto &component = dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
        if (forced)
          component.PrefabDirty = false;
      }
    }(),
    ...);
}

template <typename... Component>
static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst,
                                  Entity src, bool forced = false) {
  CopyComponentIfExists<Component...>(dst, src, forced);
}

template <typename... Component>
static void CopyComponentIfExistsAndNotDirty(Entity dst, Entity src) {
  (
    [&]() {
      if (src.HasComponent<Component>() && !dst.GetComponent<Component>().PrefabDirty)
        dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
    }(),
    ...);
}

template <typename... Component>
static void CopyComponentIfExistsAndNotDirty(ComponentGroup<Component...>, Entity dst,
                                  Entity src) {
  CopyComponentIfExists<Component...>(dst, src);
}

template <typename T>
static void CopyComponentIfExistsForEntity(entt::registry &dstRegistry,
                                           entt::registry &srcRegistry,
                                           entt::entity srcEntity,
                                           entt::entity destEntity) {
  if (srcRegistry.any_of<T>(srcEntity)) {
    auto &srcComponent = srcRegistry.get<T>(srcEntity);
    dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
  }
}

}

using namespace SceneUtil;

void Scene::RecursiveCopyTo(Ref<Scene> &target, const UUID &entityID) {
  if (!target)
    return;

  Entity srcEntityWrapper = GetEntityByUUID(entityID);
  UUID uuid = srcEntityWrapper.GetUUID();
  std::string name = srcEntityWrapper.GetName();
  UUID parent = srcEntityWrapper.GetComponent<RelationshipComponent>().Parent;
  Entity destEntityWrapper = target->CreateEntityWithUUID(uuid, name, parent);

  if (!destEntityWrapper)
    return;

  CopyComponentIfExists<
    TagComponent,
    LayerComponent,
    ObjectStateComponent,
    TransformComponent,
    CameraComponent,
    SpriteRendererComponent,
    CircleRendererComponent,
    Rigidbody2DComponent,
    BoxCollider2DComponent,
    CircleCollider2DComponent,
    ScriptComponentContainer,
    PrefabLinkComponent,
    RectTransformComponent,
    ButtonVisualComponent,
    PanelComponent,
    CanvasComponent,
    CanvasGroupComponent,
    SelectableComponent,
    EventListenerComponent
    >(destEntityWrapper, srcEntityWrapper, false);

  auto &rc = srcEntityWrapper.GetComponent<RelationshipComponent>();
  for (auto &child : rc.Children) {
    RecursiveCopyTo(target, child);
  }
}

void Scene::CopyTo(Ref<Scene> &target) {
  CE_PROFILE_FUNCTION();

  target->m_ViewportWidth = m_ViewportWidth;
  target->m_ViewportHeight = m_ViewportHeight;
  target->m_Handle = m_Handle;

  for (auto &parent : m_RootEntities) {
    RecursiveCopyTo(target, parent);
  }

  target->RegisterParents();
  target->UpdateAllPrefabs();
}

void Scene::OnUpdateEditor(Time ts, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();

  if (!m_IsRunning)
    ExecuteSceneCommandQueue();

  Entity cam = GetPrimaryCameraEntity();
  m_UIVirtualCanvas = !cam || cam.GetComponent<CameraComponent>().Camera.GetProjectionType()
                                != SceneCamera::ProjectionType::Orthographic;
  glm::vec2 layoutSize{0.0f};
  if (cam && !m_UIVirtualCanvas) {
    auto &cc = cam.GetComponent<CameraComponent>();
    cc.ApplyView(cc.Resolve(m_ViewportWidth, m_ViewportHeight));

    // Stable design size, independent of the editor panel.
    glm::vec2 ref{1920.0f, 1080.0f};
    for (auto id : m_Registry.view<CanvasComponent>()) {
      ref = m_Registry.get<CanvasComponent>(id).ReferenceResolution;
      break;
    }
    ref = glm::max(ref, glm::vec2(1.0f));

    switch (cc.AspectMode) {
      case CameraAspectMode::Pixels: layoutSize = glm::max(cc.Resolution, glm::vec2(1.0f)); break;
      case CameraAspectMode::Ratio:
        layoutSize = {ref.y * cc.AspectRatio.x / std::max(cc.AspectRatio.y, 0.001f), ref.y};
        break;
      default: layoutSize = ref; break;
    }
  }

  UpdateUILayout(layoutSize);
  Renderer2D::BeginScene(camera);
  const auto &rootEntities = GetRootEntities();
  for (auto entityID : rootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    if (rootEntity) {
      // Pass identity matrix (1.0) as the base parent matrix for roots
      DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
  }
  Renderer2D::EndScene();

  DrawUICanvases(camera);
}

void Scene::OnUpdateMainCameraPreview(Time ts, const glm::vec2 &viewSize) {
  CE_PROFILE_FUNCTION();

  glm::vec2 viewSizeT = viewSize;
  if (viewSize.x <= 0.0f || viewSize.y <= 0.0f)
    viewSizeT = {(float)m_ViewportWidth, (float)m_ViewportHeight};

  UpdateTransforms();

  CameraComponent *mainCamera = nullptr;
  glm::mat4 cameraTransform{1.0f};
  CameraViewInfo view;
  m_UIViewOrigin = {0.0f, 0.0f};
  {
    auto cameras = m_Registry.view<CameraComponent>();
    for (auto entity : cameras) {
      auto &cc = cameras.get<CameraComponent>(entity);
      if (cc.Primary && cc.Active) {
        Entity e{entity, this};
        mainCamera = &cc;
        cameraTransform = e.GetWorldTransform();
        view = cc.Resolve((uint32_t)viewSizeT.x, (uint32_t)viewSizeT.y);
        cc.ApplyView(view);
        m_UIViewOrigin = glm::vec2(view.Origin);
        break;
      }
    }
  }

  // The UI lays out against the camera's (possibly letterboxed) output, not the whole panel.
  m_UIVirtualCanvas = false;
  UpdateUILayout(mainCamera ? glm::vec2(view.Size) : viewSizeT);

  if (mainCamera) {
    if (mainCamera->ClearMode == CameraClearMode::SolidColor)
      RenderCommand::ClearRegion(view.Origin.x, view.Origin.y, view.Size.x, view.Size.y,
                                 mainCamera->BackgroundColor);
    RenderCommand::SetViewport(view.Origin.x, view.Origin.y, view.Size.x, view.Size.y);

    Renderer2D::BeginScene(mainCamera->Camera.GetProjection(), cameraTransform);
    const auto &rootEntities = GetRootEntities();
    for (auto entityID : rootEntities) {
      Entity rootEntity = GetEntityByUUID(entityID);
      if (rootEntity)
        DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
    Renderer2D::EndScene();
  }

  DrawUICanvases(); // ortho over m_UILayoutSize, so it fills the current viewport

  RenderCommand::SetViewport(0, 0, (uint32_t)viewSizeT.x, (uint32_t)viewSizeT.y);
}
void Scene::OnPauseStart() { m_IsPaused = true; }

void Scene::OnPauseStop() { m_IsPaused = false; }

void Scene::OnRuntimeStart() {
  m_IsRunning = true;
  OnPhysics2DStart();
  OnScriptStart();
}

void Scene::OnUpdateRuntime(Time time, const glm::vec2 &viewSize) {
  CE_PROFILE_FUNCTION();
  ExecuteSceneCommandQueue();

  float frameTime = time.deltaTime();

  if (frameTime > 0.25f)
    frameTime = 0.25f;

  if (!m_IsPaused) {
    m_Accumulator += frameTime;

    while (m_Accumulator >= m_FixedTimeStep) {
      // Loads the delete command to run the next update frame, allowing the physics engine to catch up
      DeleteRuntimeEntities();

      // Update Scripts
      ScriptEngine::GetMutable().OnFixedUpdate(Time(m_FixedTimeStep));
      // Physics 2D
      OnPhysics2DUpdate(Time(m_FixedTimeStep));

      m_Accumulator -= m_FixedTimeStep;
    }

    ScriptEngine::GetMutable().OnUpdate(time.deltaTime());
  }

  glm::vec2 viewSizeT = viewSize;
  if (viewSize.x <= 0.0f || viewSize.y <= 0.0f)
    viewSizeT = {(float)m_ViewportWidth, (float)m_ViewportHeight};

  CameraComponent *mainCamera = nullptr;
  glm::mat4 cameraTransform{1.0f};
  CameraViewInfo view;
  m_UIViewOrigin = {0.0f, 0.0f};
  {
    auto cameras = m_Registry.view<CameraComponent>();
    for (auto entity : cameras) {
      auto &cc = cameras.get<CameraComponent>(entity);
      if (cc.Primary && cc.Active) {
        Entity e{entity, this};
        mainCamera = &cc;
        cameraTransform = e.GetWorldTransform();
        view = cc.Resolve((uint32_t)viewSizeT.x, (uint32_t)viewSizeT.y);
        cc.ApplyView(view);
        m_UIViewOrigin = glm::vec2(view.Origin);
        break;
      }
    }
  }

  m_UIVirtualCanvas = false;
  UpdateUILayout(mainCamera ? glm::vec2(view.Size) : viewSizeT);
  UpdateUITints(time.deltaTime());

  if (mainCamera) {
    if (mainCamera->ClearMode == CameraClearMode::SolidColor)
      RenderCommand::ClearRegion(view.Origin.x, view.Origin.y, view.Size.x, view.Size.y,
                                 mainCamera->BackgroundColor);
    RenderCommand::SetViewport(view.Origin.x, view.Origin.y, view.Size.x, view.Size.y);

    Renderer2D::BeginScene(mainCamera->Camera.GetProjection(), cameraTransform);
    const auto &rootEntities = GetRootEntities();
    for (auto entityID : rootEntities) {
      Entity rootEntity = GetEntityByUUID(entityID);
      if (rootEntity)
        DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
    Renderer2D::EndScene();
  }

  DrawUICanvases();

  RenderCommand::SetViewport(0, 0, (uint32_t)viewSize.x, (uint32_t)viewSize.y);
}

void Scene::OnViewportResize(uint32_t width, uint32_t height) {
  if (m_ViewportWidth == width && m_ViewportHeight == height)
    return;

  m_ViewportWidth = width;
  m_ViewportHeight = height;

  // Resize non-fixed aspect ratios
  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    auto &cameraComponent = view.get<CameraComponent>(entity);
    cameraComponent.Camera.SetViewportSize(width, height);
  }
}

void Scene::ShutdownRuntime() {
  if (!m_IsRunning) {
    return;
  }
  m_IsRunning = false;
  ScriptEngine::GetMutable().OnRuntimeStop();

  Physics2D::Get().Stop();

  // Clear any maps that reference entities
  m_EntityMap.clear();

  // Finally clear the registry
  m_Registry.clear();
}

void Scene::OnRuntimeStop() {
  m_IsRunning = false;
  OnPhysics2DStop();
  ScriptEngine::GetMutable().OnRuntimeStop();
  
  auto view = m_Registry.view<ScriptComponentContainer>();
  for (auto entityID : view) {
    auto &container = view.get<ScriptComponentContainer>(entityID);
    for (auto &sc : container.Scripts) {
      sc.Loaded = false;
    }
  }
}

void Scene::OnSimulationStart() { OnPhysics2DStart(); }

void Scene::OnUpdateSimulation(Time time, EditorCamera &camera) {
  CE_PROFILE_FUNCTION();
  ExecuteSceneCommandQueue();
  UpdateTransforms();
  if (!m_IsPaused) {
    // Physics 2D
    Physics2D::Get().Update(time);
  }
  Renderer2D::BeginScene(camera);
  const auto &rootEntities = GetRootEntities();
  for (auto entityID : rootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    if (rootEntity) {
      // Pass identity matrix (1.0) as the base parent matrix for roots
      DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
    }
  }
  Renderer2D::EndScene();

  // Render 2D
  Camera *mainCamera = nullptr;
  glm::mat4 cameraTransform;
  {
    auto view = m_Registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
      auto [transform, camera] =
          view.get<TransformComponent, CameraComponent>(entity);

      Entity e = {entity, this};

      if (camera.Primary) {
        mainCamera = &camera.Camera;
        cameraTransform = e.GetWorldTransform();
        break;
      }
    }
  }

  if (mainCamera) {
    Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);
    // Draw Sprites
    const auto &rootEntities = GetRootEntities();
    for (auto entityID : rootEntities) {
      Entity rootEntity = GetEntityByUUID(entityID);
      if (rootEntity) {
        // Pass identity matrix (1.0) as the base parent matrix for roots
        DrawEntityRecursive(rootEntity, glm::mat4(1.0f));
      }
    }
    Renderer2D::EndScene();
  }
}

void Scene::OnSimulationStop() { OnPhysics2DStop(); }

void Scene::OnPhysics2DStart() {
  CE_PROFILE_FUNCTION();

  UpdateTransforms();

  Physics2D::Get().Start(this);
}

void Scene::OnPhysics2DUpdate(Time ts) { Physics2D::Get().Update(ts); }

void Scene::OnPhysics2DStop() { Physics2D::Get().Stop(); }

void Scene::OnScriptStart() { ScriptEngine::GetMutable().LoadRuntimeScripts(); }

void Scene::UpdateTransforms() {
  for (auto entityID : m_RootEntities) {
    Entity rootEntity = GetEntityByUUID(entityID);
    // Pass an identity matrix as the "parent" matrix for roots
    CalculateTransformRecursive(rootEntity, glm::mat4(1.0f));
  }
}

void Scene::CalculateTransformRecursive(Entity entity,
                                        const glm::mat4 &parentWorldMatrix) {
  auto &tc = entity.GetComponent<TransformComponent>();
  auto &rc = entity.GetComponent<RelationshipComponent>();

  // 1. Calculate true world matrix via matrix multiplication
  glm::mat4 worldMatrix = parentWorldMatrix * tc.GetLocalTransform();

  // 2. Decompose back into World fields for rendering and physics
  Math::DecomposeTransform(worldMatrix, tc.WorldPosition, tc.WorldRotation,
                           tc.WorldScale);

  // 3. Pass this world matrix down to all children
  for (auto childUUID : rc.Children) {
    Entity childEntity = GetEntityByUUID(childUUID);
    if (childEntity) {
      CalculateTransformRecursive(childEntity, worldMatrix);
    }
  }
}

void Scene::UpdateUILayout(const glm::vec2 &sizeOverride) {
  m_UIRects.clear();
  m_UIDrawList.clear();
  m_UIDrawIndex.clear();

  glm::vec2 viewportSize;
  if (m_UIVirtualCanvas) {
    // Fixed design resolution, independent of the editor panel size.
    viewportSize = {1920.0f, 1080.0f};
    for (auto id : m_Registry.view<CanvasComponent>()) {
      viewportSize = m_Registry.get<CanvasComponent>(id).ReferenceResolution;
      break; // first canvas defines the virtual screen
    }
    viewportSize = glm::max(viewportSize, glm::vec2(1.0f));
  } else {
    if (sizeOverride.x > 0.0f && sizeOverride.y > 0.0f) {
      viewportSize = sizeOverride;
    } else {
      if (m_ViewportWidth == 0 || m_ViewportHeight == 0)
        return;
      viewportSize = {(float)m_ViewportWidth, (float)m_ViewportHeight};
    }
  }
  m_UILayoutSize = viewportSize;

  const UIScreenRect rootRect{glm::vec2{0.0f}, viewportSize};

  struct Bucket { int32_t SortOrder; std::vector<UIDrawItem> Items; };
  std::vector<Bucket> buckets;

  auto canvasView = m_Registry.view<CanvasComponent>();
  for (auto id : canvasView) {
    Entity canvasEntity{id, this};
    //if (!IsEntityActiveInHierarchy(canvasEntity))
    //  continue;

    const auto &canvas = canvasView.get<CanvasComponent>(id);
    const float scale = canvas.ComputeScaleFactor(viewportSize);
    m_UIRects[canvasEntity.GetUUID()] = rootRect;

    buckets.push_back({canvas.SortOrder, {}});
    std::vector<UIDrawItem> &items = buckets.back().Items; // no push_back until we're done with it

    if (!canvasEntity.HasComponent<RelationshipComponent>())
      continue;

    // A CanvasGroup on the canvas entity itself applies to everything under it.
    const UIGroupState rootGroup = SceneUtil::ResolveGroup(canvasEntity, UIGroupState{});

    const auto children = canvasEntity.GetComponent<RelationshipComponent>().Children;
    for (UUID childUUID : children)
    if (Entity child = GetEntityByUUID(childUUID))
      UpdateUINodeRecursive(child, rootRect, scale, rootGroup, 0, items);
  }

  std::stable_sort(buckets.begin(), buckets.end(), [](const Bucket &a, const Bucket &b) { return a.SortOrder < b.SortOrder; });
  for (auto &b : buckets)
    m_UIDrawList.insert(m_UIDrawList.end(), b.Items.begin(), b.Items.end());

  for (uint32_t i = 0; i < m_UIDrawList.size(); ++i)
    m_UIDrawIndex[m_UIDrawList[i].Entity] = i;
}


void Scene::UpdateUINodeRecursive(Entity entity, const UIScreenRect &parentRect, float canvasScale,
                                  const UIGroupState &parentGroup, UUID inheritedTint,
                                  std::vector<UIDrawItem> &out) {
  if (!entity)
    return;

  if (entity.HasComponent<ObjectStateComponent>() &&
      !entity.GetComponent<ObjectStateComponent>().Active)
    return;

  const UIGroupState group = SceneUtil::ResolveGroup(entity, parentGroup);

  // Which selectable's tint colors this node's graphics, and its children's.
  UUID selfTint = inheritedTint, childTint = inheritedTint;
  if (entity.HasComponent<SelectableComponent>()) {
    selfTint = entity.GetUUID();
    if (entity.GetComponent<SelectableComponent>().TintChildren)
      childTint = selfTint;
  }

  UIScreenRect currentRect = parentRect;
  if (entity.HasComponent<RectTransformComponent>()) {
    currentRect = ComputeNodeRect(entity.GetComponent<RectTransformComponent>(), parentRect, canvasScale);
    m_UIRects[entity.GetUUID()] = currentRect;
    // Pre-order: parents draw before children.
    out.push_back({entity.GetUUID(), canvasScale, group.Alpha, group.Interactable,
                   group.BlocksRaycasts, selfTint});
  }

  if (!entity.HasComponent<RelationshipComponent>())
    return;

  for (UUID childUUID : entity.GetComponent<RelationshipComponent>().Children)
    if (Entity child = GetEntityByUUID(childUUID))
      UpdateUINodeRecursive(child, currentRect, canvasScale, group, childTint, out);
}

void Scene::UpdateUIInput(float dt) {
  CE_PROFILE_FUNCTION();
  auto &in = m_UIInput;

  // Take this frame's input and consume the one-shots.
  const UIInputFrame frame = in.Frame;
  in.Frame.Scroll = {0.0f, 0.0f};
  in.Frame.Move = UINavDirection::None;
  in.Frame.Submit = in.Frame.Cancel = in.Frame.TabNext = in.Frame.TabPrev = false;
  in.Time += dt;

  ValidateUIInputState();

  // Hit-test against last frame's layout (this runs before UpdateUILayout).
  const glm::vec2 pos = frame.PointerValid ? frame.PointerPos : in.LastPointerPos;
  in.LastPointerPos = pos;
  in.RawHit = frame.PointerValid ? RaycastUI(pos) : UUID(0);

  SetUIHovered(ResolveUITarget(in.RawHit, UIMask::Hover));

  for (int b = 0; b < 3; ++b)
    ProcessUIPointerButton(b, frame.Buttons[b], frame.PointerValid, pos);

  if (frame.Scroll != glm::vec2(0.0f)) {
    if (UUID t = ResolveUITarget(in.RawHit, UIMask::Scroll))
      SendUIEvent(UIEventType::Scroll, t, 0, frame.Scroll);
  }

  ProcessUINavigation(frame);
}

void Scene::UpdateUITints(float dt) {
  const auto &in = m_UIInput;
  const auto &left = in.Btn[0];

  for (const UIDrawItem &item : m_UIDrawList) {
    Entity e = GetEntityByUUID(item.Entity);
    if (!e || !e.HasComponent<SelectableComponent>())
      continue;
    const auto &s = e.GetComponent<SelectableComponent>();

    // Same priority as Unity: Disabled > Pressed > Selected > Highlighted > Normal
    glm::vec4 target{1.0f};
    if (s.Transition == UITransition::ColorTint) {
      if (!s.Interactable || !item.Interactable)                     target = s.DisabledTint;
      else if (left.Down && !left.Ignored && left.PressTarget == item.Entity) target = s.PressedTint;
      else if (in.Selected == item.Entity)                           target = s.SelectedTint;
      else if (in.Hovered == item.Entity)                            target = s.HighlightedTint;
      else                                                           target = s.NormalTint;
    }

    auto [it, inserted] = m_UITints.try_emplace(item.Entity, target); // snap on first sight
    if (inserted)
      continue;

    glm::vec4 &cur = it->second;
    if (s.FadeDuration <= 0.0f)
      cur = target;
    else
      cur += (target - cur) * std::min(dt / s.FadeDuration, 1.0f);
  }

  for (auto it = m_UITints.begin(); it != m_UITints.end();)
    it = FindUIDrawItem(it->first) ? std::next(it) : m_UITints.erase(it);
}

void Scene::ValidateUIInputState() {
  auto &in = m_UIInput;
  if (in.Hovered != 0 && !FindUIDrawItem(in.Hovered))
    SetUIHovered(0);

  if (in.Selected != 0) {
    Entity sel = GetEntityByUUID(in.Selected);
    const UIDrawItem *item = FindUIDrawItem(in.Selected);
    const bool ok = sel && item && item->Interactable &&
                    sel.HasComponent<SelectableComponent>() &&
                    sel.GetComponent<SelectableComponent>().Interactable;
    if (!ok)
      SetSelectedUI({});
  }

  for (auto &s : in.Btn) {
    if (s.PressTarget != 0 && !FindUIDrawItem(s.PressTarget)) s.PressTarget = 0;
    if (s.ClickTarget != 0 && !FindUIDrawItem(s.ClickTarget)) s.ClickTarget = 0;
    if (s.DragTarget != 0 && !FindUIDrawItem(s.DragTarget)) {
      s.DragTarget = 0;
      s.Dragging = false;
    }
  }
}

const UIDrawItem *Scene::FindUIDrawItem(UUID id) const {
  auto it = m_UIDrawIndex.find(id);
  return it != m_UIDrawIndex.end() ? &m_UIDrawList[it->second] : nullptr;
}

UUID Scene::RaycastUI(const glm::vec2 &p) {
  for (auto it = m_UIDrawList.rbegin(); it != m_UIDrawList.rend(); ++it) {
    if (!it->BlocksRaycasts)
      continue;
    const UIScreenRect *r = GetUIRect(it->Entity);
    if (!r)
      continue;
    if (p.x < r->Min.x || p.x > r->Max.x || p.y < r->Min.y || p.y > r->Max.y)
      continue;
    Entity e = GetEntityByUUID(it->Entity);
    if (e && SceneUtil::IsRaycastTarget(e))
      return it->Entity; // topmost raycastable graphic under the pointer
  }
  return 0;
}

UUID Scene::ResolveUITarget(UUID raw, uint32_t mask, bool needSelectable) {
  Entity cur = GetEntityByUUID(raw);
  size_t guard = m_EntityMap.size();

  while (cur && guard--) {
    if (const UIDrawItem *item = FindUIDrawItem(cur.GetUUID())) {
      const bool isSelectable = cur.HasComponent<SelectableComponent>();
      const bool handles = needSelectable ? isSelectable
                                          : (SceneUtil::HandlerMask(cur) & mask) != 0;
      if (handles) {
        bool interactable = item->Interactable;
        if (isSelectable)
          interactable = interactable && cur.GetComponent<SelectableComponent>().Interactable;
        return interactable ? cur.GetUUID() : UUID(0);
      }
    }

    const UUID parent = cur.GetComponent<RelationshipComponent>().Parent;
    if (parent == 0)
      break;
    cur = GetEntityByUUID(parent);
  }
  return 0;
}
bool Scene::SendUIEvent(UIEventType type, UUID target, int button,
                        glm::vec2 delta, int clickCount, UINavDirection dir) {
  if (!m_IsRunning || target == 0)
    return false;

  Entity e = GetEntityByUUID(target);
  if (!e || !e.HasComponent<ObjectStateComponent>() ||
      !e.GetComponent<ObjectStateComponent>().Active)
    return false; // inactive or destroyed objects get nothing

  UIEvent ev;
  ev.Type = type;
  ev.Target = target;
  ev.Position = m_UIInput.LastPointerPos;
  ev.Delta = delta;
  ev.Button = (uint8_t)button;
  ev.ClickCount = clickCount;
  ev.Direction = dir;

  // Hook point: forward to scripts here, e.g.
  // ScriptEngine::GetMutable().OnUIEvent(GetEntityByUUID(target), ev);
  return ScriptEngine::GetMutable().DispatchUIEvent(e, ev);
}

void Scene::SetUIHovered(UUID next) {
  auto &in = m_UIInput;
  if (in.Hovered == next)
    return;

  const UUID prev = in.Hovered;
  in.Hovered = next;
  if (prev != 0 && GetEntityByUUID(prev)) // skip if it was destroyed
    SendUIEvent(UIEventType::PointerExit, prev);
  if (next != 0)
    SendUIEvent(UIEventType::PointerEnter, next);
}

void Scene::ProcessUIPointerButton(int b, bool down, bool pointerValid,
                                   const glm::vec2 &pos) {
  auto &in = m_UIInput;
  auto &s = in.Btn[b];

  const bool pressedNow = down && !s.Down;
  const bool releasedNow = !down && s.Down;
  s.Down = down;

  if (pressedNow) {
    // A press that starts outside the game view never becomes ours, even if the
    // pointer later drags into it.
    s.Ignored = !pointerValid;
    if (s.Ignored)
      return;

    const UUID raw = in.RawHit;
    s.PressTarget = ResolveUITarget(raw, UIMask::Press);
    s.ClickTarget = ResolveUITarget(raw, UIMask::Click);
    s.DragTarget  = ResolveUITarget(raw, UIMask::Drag);
    s.PressPos = s.LastPos = pos;
    s.Dragging = false;

    // Left press moves selection. Pressing anything non-selectable clears it.
    if (b == 0)
      SetSelectedUI(GetEntityByUUID(ResolveUITarget(raw, 0, true)));

    SendUIEvent(UIEventType::PointerDown, s.PressTarget, b);
    return;
  }

  if (down) {
    if (s.Ignored)
      return;

    const glm::vec2 delta = pos - s.LastPos;
    s.LastPos = pos;

    if (!s.Dragging && s.DragTarget != 0 &&
        glm::distance(pos, s.PressPos) > m_UIDragThreshold) {
      s.Dragging = true;
      SendUIEvent(UIEventType::BeginDrag, s.DragTarget, b);
    }
    // The drag target keeps receiving events even when the pointer leaves it.
    if (s.Dragging && delta != glm::vec2(0.0f))
      SendUIEvent(UIEventType::Drag, s.DragTarget, b, delta);
    return;
  }

  if (!releasedNow)
    return;

  if (s.Ignored) {
    s.Ignored = false;
    return;
  }

  const UUID clickNow = ResolveUITarget(in.RawHit, UIMask::Click);

  if (s.PressTarget != 0 && GetEntityByUUID(s.PressTarget))
    SendUIEvent(UIEventType::PointerUp, s.PressTarget, b);

  if (s.Dragging) {
    if (s.DragTarget != 0)
      SendUIEvent(UIEventType::EndDrag, s.DragTarget, b);
  } else if (s.ClickTarget != 0 && clickNow == s.ClickTarget) {
    // Pressed and released on the same handler: a click. Count rapid repeats.
    const bool repeat = s.LastClickTarget == s.ClickTarget &&
                        (in.Time - s.LastClickTime) <= m_UIDoubleClickTime &&
                        glm::distance(pos, s.LastClickPos) <= m_UIDragThreshold;
    s.ClickCount = repeat ? s.ClickCount + 1 : 1;
    s.LastClickTarget = s.ClickTarget;
    s.LastClickTime = in.Time;
    s.LastClickPos = pos;
    SendUIEvent(UIEventType::PointerClick, s.ClickTarget, b, {0.0f, 0.0f}, s.ClickCount);
  }

  s.PressTarget = s.ClickTarget = s.DragTarget = 0;
  s.Dragging = false;
}

void Scene::ProcessUINavigation(const UIInputFrame &frame) {
  auto &in = m_UIInput;
  Entity selected = GetEntityByUUID(in.Selected);

  if (frame.Move != UINavDirection::None) {
    // The selected widget gets first refusal (e.g. a slider consuming Left/Right).
    const bool consumed = selected &&
        SendUIEvent(UIEventType::Move, in.Selected, 0, {0.0f, 0.0f}, 0, frame.Move);
    if (!consumed) {
      Entity next = selected ? FindNavigationTarget(selected, frame.Move)
                             : FindTabTarget(Entity{}, true);
      if (next) { SetSelectedUI(next); selected = next; }
    }
  }

  if (frame.TabNext || frame.TabPrev) {
    if (Entity next = FindTabTarget(selected, frame.TabNext)) {
      SetSelectedUI(next);
      selected = next;
    }
  }

  if (frame.Submit && selected) SendUIEvent(UIEventType::Submit, selected.GetUUID());
  if (frame.Cancel && selected) SendUIEvent(UIEventType::Cancel, selected.GetUUID());
}

std::vector<Entity> Scene::CollectNavigableUI() {
  std::vector<Entity> out;
  for (const UIDrawItem &item : m_UIDrawList) {
    if (!item.Interactable)
      continue;
    Entity e = GetEntityByUUID(item.Entity);
    if (!e || !e.HasComponent<SelectableComponent>())
      continue;
    const auto &s = e.GetComponent<SelectableComponent>();
    if (s.Interactable && s.Navigable)
      out.push_back(e);
  }
  return out; // draw order = hierarchy order, which is also the Tab order}
}
Entity Scene::FindNavigationTarget(Entity from, UINavDirection dir) {
  const UIScreenRect *fromRect = GetUIRect(from.GetUUID());
  if (!fromRect)
    return {};

  glm::vec2 d{0.0f};
  switch (dir) {
    case UINavDirection::Left:  d = {-1.0f, 0.0f}; break;
    case UINavDirection::Right: d = { 1.0f, 0.0f}; break;
    case UINavDirection::Up:    d = { 0.0f, 1.0f}; break;
    case UINavDirection::Down:  d = { 0.0f, -1.0f}; break;
    default: return {};
  }

  const glm::vec2 origin = (fromRect->Min + fromRect->Max) * 0.5f;
  float bestScore = 0.0f;
  Entity best;

  for (Entity cand : CollectNavigableUI()) {
    if (cand.GetUUID() == from.GetUUID())
      continue;
    const UIScreenRect *r = GetUIRect(cand.GetUUID());
    if (!r)
      continue;

    const glm::vec2 v = (r->Min + r->Max) * 0.5f - origin;
    const float along = glm::dot(d, v);
    const float dist2 = glm::dot(v, v);
    if (along <= 0.0f || dist2 <= 0.0f)
      continue; // behind us, or exactly on top of us

    const float score = along / dist2;
    if (score > bestScore) {
      bestScore = score;
      best = cand;
    }
  }
  return best;
}

Entity Scene::FindTabTarget(Entity from, bool forward) {
  const auto list = CollectNavigableUI();
  if (list.empty())
    return {};

  size_t i = list.size();
  if (from) {
    for (size_t k = 0; k < list.size(); ++k)
      if (list[k].GetUUID() == from.GetUUID()) { i = k; break; }
  }
  if (i == list.size())
    return forward ? list.front() : list.back();

  const size_t n = list.size();
  return list[forward ? (i + 1) % n : (i + n - 1) % n]; // wraps
}

glm::vec4 Scene::UITintFor(const UIDrawItem &item) const {
  glm::vec4 tint{1.0f};
  if (item.TintSource != 0) {
    auto it = m_UITints.find(item.TintSource);
    if (it != m_UITints.end())
      tint = it->second;
  }
  tint.a *= item.Alpha; // CanvasGroup alpha
  return tint;
}

void Scene::DrawUIList(const glm::mat4 &toWorld) {
  for (const UIDrawItem &item : m_UIDrawList) {
    Entity e = GetEntityByUUID(item.Entity);
    const UIScreenRect *rect = GetUIRect(item.Entity);
    if (!e || !rect)
      continue;

    const int id = (int)e.GetEntityID();
    const glm::vec4 tint = UITintFor(item);

    if (e.HasComponent<PanelComponent>()) {
      const auto &p = e.GetComponent<PanelComponent>();
      DrawUIQuad(*rect, p.Color * tint, p.Texture, id, toWorld);
    }
    if (e.HasComponent<ButtonVisualComponent>()) {
      const auto &b = e.GetComponent<ButtonVisualComponent>();
      DrawUIQuad(*rect, b.NormalColor * tint, b.Texture, id, toWorld);
    }
  }
}

void Scene::DrawUICanvases(EditorCamera &camera) {
  CE_PROFILE_FUNCTION();

  if (m_UIDrawList.empty())
    return;

  glm::mat4 canvasToWorld;
  if (!ComputeCanvasToWorld(canvasToWorld))
    return; // no primary camera: nothing to lock onto

  Renderer2D::BeginScene(camera);
  DrawUIList(canvasToWorld);
  Renderer2D::EndScene();
}

void Scene::DrawUICanvases() {
  CE_PROFILE_FUNCTION();

  if (m_UIDrawList.empty())
    return;

  const glm::mat4 projection =
      glm::ortho(0.0f, m_UILayoutSize.x, 0.0f, m_UILayoutSize.y, -1.0f, 1.0f);

  Renderer2D::BeginScene(projection, glm::mat4(1.0f));
  DrawUIList(glm::mat4(1.0f));
  Renderer2D::EndScene();
}



bool Scene::ComputeCanvasToWorld(glm::mat4 &out) {
  Entity camEntity = GetPrimaryCameraEntity();

  if (m_UIVirtualCanvas || !camEntity) {
    // Canvas centered on the world origin, in the z = 0 plane.
    const float s = 1.0f / kVirtualPixelsPerUnit;
    out = glm::translate(glm::mat4(1.0f), glm::vec3(-m_UILayoutSize * 0.5f * s, 0.0f)) *
          glm::scale(glm::mat4(1.0f), glm::vec3(s, s, 1.0f));
    return true;
  }

  auto &cam = camEntity.GetComponent<CameraComponent>().Camera;
  const glm::mat4 invProj = glm::inverse(cam.GetProjection());

  auto unproject = [&](glm::vec2 ndc) {
    const glm::vec4 p = invProj * glm::vec4(ndc, 0.0f, 1.0f);
    return glm::vec2(p) / p.w;
  };

  // Visible rectangle in camera space
  const glm::vec2 lo = unproject({-1.0f, -1.0f});
  const glm::vec2 hi = unproject({ 1.0f,  1.0f});

  const float s = (hi.y - lo.y) / m_UILayoutSize.y;
  const glm::vec2 center = (lo + hi) * 0.5f;
  const glm::vec2 origin = center - m_UILayoutSize * s * 0.5f;

  out = camEntity.GetWorldTransform() *
        glm::translate(glm::mat4(1.0f), glm::vec3(origin, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(s, s, 1.0f));
  return true;
}

float Scene::GetUIScale(const UUID &id) const {
  const UIDrawItem *item = FindUIDrawItem(id);
  return (item && item->Scale > 0.0001f) ? item->Scale : 1.0f;
}

void Scene::DrawUIQuad(const UIScreenRect &rect, const glm::vec4 &color,
                       AssetHandle texture, int entityID, const glm::mat4 &canvasToWorld) {
  const glm::vec2 size = rect.Max - rect.Min;
  if (size.x <= 0.0f || size.y <= 0.0f)
    return;

  const glm::vec2 center = (rect.Min + rect.Max) * 0.5f;

  // Assumes Renderer2D's unit quad is centered on the origin.
  const glm::mat4 transform =
    canvasToWorld *
    glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f)) *
    glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

  // Reuse the sprite path so texture lookup and batching behave identically.
  SpriteRendererComponent sprite;
  sprite.Color = color;
  sprite.Texture = texture;

  Renderer2D::DrawSprite(transform, sprite, entityID);
}
Entity Scene::FindOrCreateCanvas() {
  auto view = m_Registry.view<CanvasComponent>();
  for (auto e : view)
    return Entity{e, this};          // first canvas found
  return CreateCanvas();
}

Entity Scene::CreateUIElement(const std::string &name, Entity parent) {
  if (!parent || !IsUnderCanvas(parent))
    parent = FindOrCreateCanvas();

  Entity element = CreateEntity(name, parent.GetUUID());

  // CreateEntityWithUUID only sets Parent, so link it into the Children list
  // or layout and drawing will never reach it.
  parent.GetComponent<RelationshipComponent>().Children.push_back(element.GetUUID());

  element.AddComponent<RectTransformComponent>(); // defaults: centered, 160x48
  return element;
}


void Scene::DeleteRuntimeEntity(Entity entity) {
  m_SceneCommandQueue.push_back([this, entity](){
    auto &rc = entity.GetComponent<RelationshipComponent>();

    for (auto &child : rc.Children) {
      DeleteRuntimeEntity(GetEntityByUUID(child));
    }

    if (entity.HasComponent<BoxCollider2DComponent>() || entity.HasComponent<CircleCollider2DComponent>()) {
      Physics2D::Get().EnableCollider(entity, false);
    }

    if (entity.HasComponent<Rigidbody2DComponent>()) {
      Physics2D::Get().EnableRigidbody(entity, false);
    }

    if (entity.HasComponent<ScriptComponentContainer>()) {
      ScriptEngine::GetMutable().DeleteEntity(entity);
    }

    m_DeletedEntities.push_back(entity.GetUUID());
  });
}

void Scene::DeleteRuntimeEntities() {
  m_SceneCommandQueue.push_back([this](){
    for (auto &entityID : m_DeletedEntities) {
      Entity e = this->GetEntityByUUID(entityID);
      if (!e)
        continue;
      m_EntityMap.erase(entityID);
      m_RootEntities.erase(std::remove(m_RootEntities.begin(), m_RootEntities.end(),
                                   entityID),
                       m_RootEntities.end());
      m_Registry.destroy(e);
    }
    m_DeletedEntities.clear();
  });
}

void Scene::DeleteEditTimeEntity(Entity entity) {
  auto &rc = entity.GetComponent<RelationshipComponent>();
  if (rc.Parent != 0) {
    UUID rootID = rc.Parent;
    // Look for parent PrefabLinkComponent
    while (rootID != 0) {
      Entity parent = entity.GetScene()->GetEntityByUUID(rootID);
      auto &prc = parent.GetComponent<RelationshipComponent>();
      if (parent.HasComponent<PrefabLinkComponent>()) {
        auto &plc = parent.GetComponent<PrefabLinkComponent>();
        std::string name = entity.GetComponent<NameComponent>().Name;
        plc.DeletedEntities.push_back(name);
        break;
      }
      rootID = prc.Parent;
    }
  }

  for (auto &child : rc.Children) {
    DeleteEditTimeEntity(GetEntityByUUID(child));
  }

  UUID entityID = entity.GetUUID();
  m_EntityMap.erase(entityID);
  m_RootEntities.erase(std::remove(m_RootEntities.begin(), m_RootEntities.end(), entityID), m_RootEntities.end());
  m_Registry.destroy(entity);
}

void Scene::DuplicateEntity(Entity entity) { PasteEntity(entity); }

Entity Scene::PasteEntity(Entity entity) {
  CE_PROFILE_FUNCTION();

  Entity newEntity = CreateEntity(entity.GetName() + "(Copy)");
  CopyComponentIfExists(AllComponents{}, newEntity, entity);
  return newEntity;
}
Entity Scene::FindEntityByName(const std::string_view &name) {
  auto view = m_Registry.view<NameComponent>();
  for (auto entity : view) {
    const NameComponent &nameComponent = view.get<NameComponent>(entity);
    if (nameComponent.Name == name) {
      return Entity{entity, this};
    }
  }
  return {};
}
Entity Scene::GetEntityByUUID(UUID entityID) {
  if (m_EntityMap.find(entityID) != m_EntityMap.end())
    return {m_EntityMap.at(entityID), this};
  return {};
}
void Scene::SetUIInput(const UIInputFrame &frame) {
  m_UIInput.Frame = frame;
}

void Scene::SetSelectedUI(Entity e) {
UUID next = 0;
  if (e && e.HasComponent<SelectableComponent>() &&
    e.GetComponent<SelectableComponent>().Interactable)
    next = e.GetUUID();

  auto &in = m_UIInput;
  if (in.Selected == next)
    return;

  const UUID prev = in.Selected;
  in.Selected = next;
  if (prev != 0 && GetEntityByUUID(prev))
    SendUIEvent(UIEventType::Deselect, prev);
  if (next != 0)
    SendUIEvent(UIEventType::Select, next);
}

Entity Scene::GetPrimaryCameraEntity() {
  CE_PROFILE_FUNCTION();

  auto view = m_Registry.view<CameraComponent>();
  for (auto entity : view) {
    const auto &cameraComponent = view.get<CameraComponent>(entity);
    if (cameraComponent.Primary)
      return Entity{entity, this};
  }
  return {};
}
Entity Scene::CreateEntity(const std::string &name, UUID parent) {
  return CreateEntityWithUUID(UUID(), name, parent);
}
Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name, UUID parent) {
  CE_PROFILE_FUNCTION();
  Entity entity = {m_Registry.create(), this};

  entity.AddComponent<IDComponent>(uuid);
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = "None";
  auto &layer = entity.AddComponent<LayerComponent>();
  layer.Layer = "Default";
  auto &entityName = entity.AddComponent<NameComponent>();
  entityName.Name = name.empty() ? "GameObject" : name;
  auto &relationship = entity.AddComponent<RelationshipComponent>();
  auto &osc = entity.AddComponent<ObjectStateComponent>();
  if (parent.uuid() != 0) { // Child
    relationship.Parent = parent;
    UUID rootParentID = relationship.Parent;
    Entity parent = GetEntityByUUID(rootParentID);
    while (rootParentID != 0) {
      parent = GetEntityByUUID(rootParentID);
      auto &prc = parent.GetComponent<RelationshipComponent>();
      rootParentID = prc.Parent;
    }

    if (parent.HasComponent<PrefabLinkComponent>()) {
      osc.PrefabAdded = true;
    }
  } else {
    m_RootEntities.push_back(uuid);
  }

  entity.AddComponent<TransformComponent>();

  m_EntityMap[uuid] = entity.GetEntityID();

  return entity;
}

Entity Scene::CreateCanvas(const std::string &name, UUID parent) {
  Entity canvas = CreateEntity(name.empty() ? "Canvas" : name, parent);
  canvas.AddComponent<CanvasComponent>();
  // A canvas fills the viewport, so give it a RectTransform for consistency
  // with the rest of the UI hierarchy.
  auto &rt = canvas.AddComponent<RectTransformComponent>();
  rt.ApplyAnchorPreset(UIAnchorPreset::StretchAll);
  rt.Size = {0.0f, 0.0f};
  return canvas;
}

Entity Scene::CreatePanel(UUID parent) {
  // Use the selected entity if it's inside a canvas, otherwise find/create one.
  Entity parentEntity = GetEntityByUUID(parent);
  Entity panel = CreateUIElement("Panel", parentEntity);

  // Panels fill their parent by default, like Unity.
  auto &rt = panel.GetComponent<RectTransformComponent>();
  rt.ApplyAnchorPreset(UIAnchorPreset::StretchAll);
  rt.Size = {0.0f, 0.0f};

  panel.AddComponent<PanelComponent>();
  return panel;
}

Entity Scene::CreateButton(UUID parent) {
  Entity parentEntity = GetEntityByUUID(parent);
  Entity button = CreateUIElement("Button", parentEntity);

  // Keep the RectTransform defaults: centered, 160x48, which is
  // Unity's default button size.
  button.AddComponent<ButtonVisualComponent>();
  button.AddComponent<SelectableComponent>();
  return button;
}

void Scene::DeleteEntity(Entity entity) {
  if (m_IsRunning) {
    DeleteRuntimeEntity(entity);
  } else {
    m_SceneCommandQueue.push_back([this, entity](){
      DeleteEditTimeEntity(entity);
    });
  }
}

void Scene::SetEntityLocked(Entity entity, bool locked) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.SelfLocked = locked;

  // Evaluate against parent hierarchy
  bool parentActive = true;
  auto& rel = entity.GetComponent<RelationshipComponent>();
  if (rel.Parent != 0) {
    Entity parent = GetEntityByUUID(rel.Parent);
    if (parent) {
      parentActive = parent.GetComponent<ObjectStateComponent>().HierarchyLocked
        && parent.GetComponent<ObjectStateComponent>().Locked;
    }
  }

  state.Locked = state.SelfLocked && state.HierarchyLocked;

  // Propagate down to all children immediately
  for (UUID childUUID : rel.Children) {
    Entity child = GetEntityByUUID(childUUID);
    if (child) {
      SetEntityChildLockedRecursive(child, state.Locked);
    }
  }
}

void Scene::SetEntityChildLockedRecursive(Entity entity, bool parentLocked) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.HierarchyLocked = parentLocked;
  bool effectiveLocked = state.SelfLocked && state.HierarchyLocked;

  auto& rel = entity.GetComponent<RelationshipComponent>();
  for (UUID childUUID : rel.Children) {
    Entity c = GetEntityByUUID(childUUID);
    if (c) {
      SetEntityChildLockedRecursive(c, effectiveLocked);
    }
  }
}

void Scene::SetEntityActive(Entity entity, bool active) {
  m_SceneCommandQueue.push_back([this, entity, active]() {
    auto& state = entity.GetComponent<ObjectStateComponent>();
    state.SelfActive = active;

    // Evaluate against parent hierarchy
    bool parentActive = true;
    auto& rel = entity.GetComponent<RelationshipComponent>();
    if (rel.Parent != 0) {
      Entity parent = GetEntityByUUID(rel.Parent);
      if (parent) {
        parentActive = parent.GetComponent<ObjectStateComponent>().HierarchyActive 
          && parent.GetComponent<ObjectStateComponent>().Active;
      }
    }

    state.HierarchyActive = parentActive;
    state.Active = state.SelfActive && state.HierarchyActive;

    if (entity.HasComponent<Rigidbody2DComponent>()) {
      if (!m_IsRunning)
        return;
      Physics2D::Get().EnableRigidbody(entity, state.Active);
    }

    if (entity.HasComponent<BoxCollider2DComponent>()) {
      if (!m_IsRunning)
        return;

      Physics2D::Get().EnableCollider(entity, state.Active);
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
      if (!m_IsRunning)
        return;
      Physics2D::Get().EnableCollider(entity, state.Active);
    }

    if (entity.HasComponent<ScriptComponentContainer>()) {
      if (!m_IsRunning)
        return;
      ScriptEngine::GetMutable().OnStartEntity(entity);
    }


    // Propagate down to all children immediately
    for (UUID childUUID : rel.Children) {
      Entity child = GetEntityByUUID(childUUID);
      if (child) {
        SetEntityChildActiveRecursive(child, state.Active);
      }
    }
  });
}

void Scene::SetEntityChildActiveRecursive(Entity entity, bool parentActive) {
  auto& state = entity.GetComponent<ObjectStateComponent>();
  state.HierarchyActive = parentActive;
  bool effectiveActive = state.SelfActive && state.HierarchyActive;

  if (entity.HasComponent<Rigidbody2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableRigidbody(entity, effectiveActive);
  }

  if (entity.HasComponent<BoxCollider2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableCollider(entity, effectiveActive);
  }

  if (entity.HasComponent<CircleCollider2DComponent>()) {
    if (!m_IsRunning)
      return;
    Physics2D::Get().EnableCollider(entity, effectiveActive);
  }

  if (entity.HasComponent<ScriptComponentContainer>()) {
    if (!m_IsRunning)
      return;
    ScriptEngine::GetMutable().OnStartEntity(entity);
  }

  auto& rel = entity.GetComponent<RelationshipComponent>();
  for (UUID childUUID : rel.Children) {
    Entity c = GetEntityByUUID(childUUID);
    if (c) {
      SetEntityChildActiveRecursive(c, effectiveActive);
    }
  }
}

Entity Scene::InstantiatePrefab(const UUID &prefabID, const UUID &parentID) {
  CE_PROFILE_FUNCTION();

  // 1. Fetch the Prefab asset via AssetManager using its UUID/handle
  Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);
  if (!prefab) {
    CE_API_WARN("Failed to load prefab with asset handle: {0}", (uint64_t)prefabID);
    auto consolePanel = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
    consolePanel->WriteWarning("Failed to load prefab with asset handle: " + std::to_string(prefabID));
    return {};
  }

  Asset::MetaData metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(prefabID);

  Ref<Scene> prefabScene = prefab->GetScene();
  auto &srcRegistry = prefabScene->GetReg();
  auto &dstRegistry = m_Registry;

  // 2. Validate prefab root entities
  const auto &prefabRoots = prefabScene->GetRootEntities();
  if (prefabRoots.empty()) {
    auto consolePanel = PanelManager::Get().GetPanel<ConsolePanel>("ConsolePanel");
    consolePanel->WriteWarning("Prefab: " + std::to_string(prefabID) + " has no root entity!");
    return {};
  }

  Entity prefabRootEntity = prefabScene->GetEntityByUUID(prefabRoots[0]);
  if (!prefabRootEntity)
    return {};

  UUID oldUUID = prefabRootEntity.GetUUID();
  UUID newUUID = UUID(); // Generate a fresh unique ID for this

  std::string name = prefabRootEntity.GetName() + " (Prefab)";
  Entity newEntity = CreateEntityWithUUID(newUUID, name, 0);
  newEntity.AddComponent<PrefabLinkComponent>(prefabID, prefabRootEntity.GetUUID(), metaData.LastUpdated);

  CopyComponentIfExists<
    TagComponent,
    LayerComponent,
    ObjectStateComponent,
    TransformComponent,
    CameraComponent,
    SpriteRendererComponent,
    CircleRendererComponent,
    Rigidbody2DComponent,
    BoxCollider2DComponent,
    CircleCollider2DComponent,
    ScriptComponentContainer,
    RectTransformComponent,
    ButtonVisualComponent,
    PanelComponent,
    CanvasComponent,
    CanvasGroupComponent,
    SelectableComponent,
    EventListenerComponent
    >(newEntity, prefabRootEntity, false);


  // Recursive lambda to clone the prefab hierarchy
  std::function<void(Entity, UUID)> cloneEntityRecursive =
    [&](Entity srcEntity, UUID currentParentUUID) {
      UUID oldUUID = srcEntity.GetUUID();
      UUID newUUID = UUID(); // Generate a fresh unique ID for this
      // instantiation instance

      std::string name = srcEntity.GetName();
      Entity newEntity = CreateEntityWithUUID(newUUID, name, currentParentUUID);


      // Copy components from source prefab entity to new destination entity
      CopyComponentIfExists<
        TagComponent,
        LayerComponent,
        ObjectStateComponent,
        TransformComponent,
        CameraComponent,
        SpriteRendererComponent,
        CircleRendererComponent,
        Rigidbody2DComponent,
        BoxCollider2DComponent,
        CircleCollider2DComponent,
        ScriptComponentContainer,
        PrefabLinkComponent,
        RectTransformComponent,
        ButtonVisualComponent,
        PanelComponent,
        CanvasComponent,
        CanvasGroupComponent,
        SelectableComponent,
        EventListenerComponent
        >(newEntity, srcEntity, false);

      // Recursively clone children
      auto &rc = srcEntity.GetComponent<RelationshipComponent>();
      for (UUID childOldUUID : rc.Children) {
        Entity childSrc = prefabScene->GetEntityByUUID(childOldUUID);
        if (childSrc) {
          cloneEntityRecursive(childSrc, newUUID);
        }
      }
    };

  auto &rc = prefabRootEntity.GetComponent<RelationshipComponent>();
  for (UUID childOldUUID : rc.Children) {
    Entity childSrc = prefabScene->GetEntityByUUID(childOldUUID);
    if (childSrc) {
      cloneEntityRecursive(childSrc, newUUID);
    }
  }

  // 4. If a valid parentID was provided, explicitly push the newly created root
  // into the parent's Children list
  if (parentID != 0) {
    Entity parentEntity = GetEntityByUUID(parentID);
    if (parentEntity && newEntity) {
      parentEntity.GetComponent<RelationshipComponent>().Children.push_back( newEntity.GetUUID());
    }
  }

  RegisterParents();
  return newEntity;
}



void Scene::DrawEntityRecursive(Entity entity,
                                const glm::mat4 &parentWorldMatrix) {
  auto &stateComponent = entity.GetComponent<ObjectStateComponent>();

  if (!stateComponent.Active) {
    return;
  }

  auto &tc = entity.GetComponent<TransformComponent>();
  auto &rc = entity.GetComponent<RelationshipComponent>();

  // 1. Compute true world matrix by multiplying parent's matrix with local
  // matrix
  glm::mat4 worldMatrix = parentWorldMatrix * tc.GetLocalTransform();

  // 2. (Optional) Update cached world components if physics or other systems
  // need them
  Math::DecomposeTransform(worldMatrix, tc.WorldPosition, tc.WorldRotation,
                           tc.WorldScale);

  // 3. Render Sprites
  if (entity.HasComponent<SpriteRendererComponent>()) {
    auto &sprite = entity.GetComponent<SpriteRendererComponent>();
    Renderer2D::DrawSprite(worldMatrix, sprite, (int)entity.GetEntityID());
  }

  // 4. Render Circles
  if (entity.HasComponent<CircleRendererComponent>()) {
    auto &circle = entity.GetComponent<CircleRendererComponent>();
    Renderer2D::DrawCircle(worldMatrix, circle, (int)entity.GetEntityID());
  }

  // 5. Recursively draw all children, passing this entity's world matrix down
  for (auto childUUID : rc.Children) {
    Entity child = GetEntityByUUID(childUUID);
    if (child) {
      DrawEntityRecursive(child, worldMatrix);
    }
  }
}

void Scene::ReparentEntity(Entity entity, Entity newParent) {
  auto &rc = entity.GetComponent<RelationshipComponent>();
  auto &tc = entity.GetComponent<TransformComponent>();

  glm::mat4 oldWorldTransform = entity.GetWorldTransform();

  if (newParent && entity.IsChild(newParent.GetUUID())) {
    return;
  }

  // 1. Detach from old parent or root list
  if (rc.Parent != 0) {
    Entity oldParent = GetEntityByUUID(rc.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(
        std::remove(oldChildren.begin(), oldChildren.end(), entity.GetUUID()),
        oldChildren.end());
  } else {
    // Remove from root entities list if you track root entities separately
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(), entity.GetUUID()),
                         m_RootEntities.end());
  }

  // 2. Attach to new parent
  if (newParent) {
    rc.Parent = newParent.GetUUID();
    auto &newParentRc = newParent.GetComponent<RelationshipComponent>();
    newParentRc.Children.push_back(entity.GetUUID());

    // 4. Calculate new local transform: Local = Inverse(NewParentWorld) *
    // OldWorld
    glm::mat4 newParentWorldMatrix = newParent.GetWorldTransform();
    glm::mat4 localMatrix =
        glm::inverse(newParentWorldMatrix) * oldWorldTransform;

    // 5. Decompose into the component's local fields
    glm::vec3 localRotation;
    Math::DecomposeTransform(localMatrix, tc.LocalPosition, localRotation,
                             tc.LocalScale);
    tc.LocalRotation = localRotation;
  } else {
    // Unparenting to root (Parent == 0)
    rc.Parent = 0;

    // When becoming a root, local transform IS the world transform
    glm::vec3 localRotation;
    Math::DecomposeTransform(oldWorldTransform, tc.LocalPosition, localRotation,
                             tc.LocalScale);
    tc.LocalRotation = localRotation;

    // Make sure to add it back to your root entities list if you maintain one
    m_RootEntities.push_back(entity.GetUUID());
  }
}

void Scene::MoveEntityBefore(Entity entityToMove, Entity targetEntity) {
  auto &moveRel = entityToMove.GetComponent<RelationshipComponent>();
  auto &targetRel = targetEntity.GetComponent<RelationshipComponent>();

  // Unlink from current parent/root
  if (moveRel.Parent != 0) {
    Entity oldParent = GetEntityByUUID(moveRel.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(),
                                  entityToMove.GetUUID()),
                      oldChildren.end());
  } else {
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(),
                                     entityToMove.GetUUID()),
                         m_RootEntities.end());
  }

  // Match target's parent
  moveRel.Parent = targetRel.Parent;

  if (targetRel.Parent != 0) {
    Entity parent = GetEntityByUUID(targetRel.Parent);
    auto &siblings = parent.GetComponent<RelationshipComponent>().Children;
    auto it =
        std::find(siblings.begin(), siblings.end(), targetEntity.GetUUID());
    siblings.insert(it, entityToMove.GetUUID());
  } else {
    auto it = std::find(m_RootEntities.begin(), m_RootEntities.end(),
                        targetEntity.GetUUID());
    m_RootEntities.insert(it, entityToMove.GetUUID());
  }
}

void Scene::MoveEntityAfter(Entity entityToMove, Entity targetEntity) {
  auto &moveRel = entityToMove.GetComponent<RelationshipComponent>();
  auto &targetRel = targetEntity.GetComponent<RelationshipComponent>();

  // Unlink from current parent/root
  if (moveRel.Parent != 0) {
    Entity oldParent = GetEntityByUUID(moveRel.Parent);
    auto &oldChildren =
        oldParent.GetComponent<RelationshipComponent>().Children;
    oldChildren.erase(std::remove(oldChildren.begin(), oldChildren.end(),
                                  entityToMove.GetUUID()),
                      oldChildren.end());
  } else {
    m_RootEntities.erase(std::remove(m_RootEntities.begin(),
                                     m_RootEntities.end(),
                                     entityToMove.GetUUID()),
                         m_RootEntities.end());
  }

  // Match target's parent
  moveRel.Parent = targetRel.Parent;

  if (targetRel.Parent != 0) {
    Entity parent = GetEntityByUUID(targetRel.Parent);
    auto &siblings = parent.GetComponent<RelationshipComponent>().Children;
    auto it =
        std::find(siblings.begin(), siblings.end(), targetEntity.GetUUID());
    if (it != siblings.end()) {
      siblings.insert(it + 1, entityToMove.GetUUID());
    }
  } else {
    auto it = std::find(m_RootEntities.begin(), m_RootEntities.end(),
                        targetEntity.GetUUID());
    if (it != m_RootEntities.end()) {
      m_RootEntities.insert(it + 1, entityToMove.GetUUID());
    }
  }
}

void Scene::RegisterParents() {
  auto view = m_Registry.view<IDComponent, RelationshipComponent>();

  // 1) Clear stale child lists first so no parent is wiped mid-build
  for (auto entity : view) {
    view.get<RelationshipComponent>(entity).Children.clear();
  }

  // 2) Gather and reverse to preserve forward creation order
  std::vector<entt::entity> orderedEntities(view.begin(), view.end());
  std::reverse(orderedEntities.begin(), orderedEntities.end());

  // 3) Re-link children to parents
  for (auto entity : orderedEntities) {
    auto [id, relation] = view.get<IDComponent, RelationshipComponent>(entity);
    if (relation.Parent != 0) {
      if (Entity parent = GetEntityByUUID(relation.Parent)) {
        parent.GetComponent<RelationshipComponent>().Children.push_back(id.ID);
      }
    }
  }
}

void Scene::SyncEntityComponentsFromPrefab(Entity dstEntity, Entity srcPrefabEntity, bool preserveInstanceTransform, bool forceUpdate) {
  TransformComponent savedTransform;
  bool savedSelfActive = true;
  if (dstEntity.HasComponent<TransformComponent>()) {
    savedTransform = dstEntity.GetComponent<TransformComponent>();
  }
  if (dstEntity.HasComponent<ObjectStateComponent>()) {
    savedSelfActive = dstEntity.GetComponent<ObjectStateComponent>().SelfActive;
  }

  // Copy/overwrite components present on the prefab entity (preserving IDComponent and RelationshipComponent)
  if (!forceUpdate) {
    CopyComponentIfExistsAndNotDirty<
      TagComponent,
      LayerComponent,
      ObjectStateComponent,
      TransformComponent,
      CameraComponent,
      SpriteRendererComponent,
      CircleRendererComponent,
      Rigidbody2DComponent,
      BoxCollider2DComponent,
      CircleCollider2DComponent,
      ScriptComponentContainer,
      RectTransformComponent,
      ButtonVisualComponent,
      PanelComponent,
      CanvasComponent,
      CanvasGroupComponent,
      SelectableComponent,
      EventListenerComponent
      >(dstEntity, srcPrefabEntity);
  } else {
    CopyComponentIfExists<
      TagComponent,
      LayerComponent,
      ObjectStateComponent,
      TransformComponent,
      CameraComponent,
      SpriteRendererComponent,
      CircleRendererComponent,
      Rigidbody2DComponent,
      BoxCollider2DComponent,
      CircleCollider2DComponent,
      ScriptComponentContainer,
      RectTransformComponent,
      ButtonVisualComponent,
      PanelComponent,
      CanvasComponent,
      CanvasGroupComponent,
      SelectableComponent,
      EventListenerComponent
      >(dstEntity, srcPrefabEntity, forceUpdate);
  }

  if (preserveInstanceTransform && dstEntity.HasComponent<TransformComponent>()) {
    dstEntity.GetComponent<TransformComponent>() = savedTransform;
  }

  if (dstEntity.HasComponent<ObjectStateComponent>()) {
    dstEntity.GetComponent<ObjectStateComponent>().SelfActive = savedSelfActive;
  }

  if (dstEntity.HasComponent<ScriptComponentContainer>()) {
    ScriptEngine::GetMutable().CopyScriptFields(srcPrefabEntity, dstEntity);
  }

  // Remove components that exist on dstEntity but were removed from srcPrefabEntity
  auto removeIfMissingOnPrefab = [&]<typename T>() {
    if constexpr (!std::is_same_v<T, IDComponent> &&
                  !std::is_same_v<T, RelationshipComponent> &&
                  !std::is_same_v<T, PrefabLinkComponent>) {
      if (dstEntity.HasComponent<T>() && !srcPrefabEntity.HasComponent<T>()) {
        dstEntity.RemoveComponent<T>();
      }
    }
  };

  removeIfMissingOnPrefab.template operator()<TagComponent>();
  removeIfMissingOnPrefab.template operator()<LayerComponent>();
  removeIfMissingOnPrefab.template operator()<ObjectStateComponent>();
  removeIfMissingOnPrefab.template operator()<TransformComponent>();
  removeIfMissingOnPrefab.template operator()<CameraComponent>();
  removeIfMissingOnPrefab.template operator()<SpriteRendererComponent>();
  removeIfMissingOnPrefab.template operator()<CircleRendererComponent>();
  removeIfMissingOnPrefab.template operator()<Rigidbody2DComponent>();
  removeIfMissingOnPrefab.template operator()<BoxCollider2DComponent>();
  removeIfMissingOnPrefab.template operator()<CircleCollider2DComponent>();
  removeIfMissingOnPrefab.template operator()<ScriptComponentContainer>();
  removeIfMissingOnPrefab.template operator()<RectTransformComponent>();
  removeIfMissingOnPrefab.template operator()<ButtonVisualComponent>();
  removeIfMissingOnPrefab.template operator()<PanelComponent>();
  removeIfMissingOnPrefab.template operator()<CanvasComponent>();
  removeIfMissingOnPrefab.template operator()<CanvasGroupComponent>();
  removeIfMissingOnPrefab.template operator()<SelectableComponent>();
  removeIfMissingOnPrefab.template operator()<EventListenerComponent>();
}

void Scene::SyncPrefabHierarchyRecursive(Entity dstEntity, Entity srcPrefabEntity, AssetHandle prefabID, uint64_t prefabVersion, bool forceUpdate) {
  if (!dstEntity || !srcPrefabEntity)
    return;

  SyncEntityComponentsFromPrefab(dstEntity, srcPrefabEntity, true, forceUpdate);

  if (dstEntity.HasComponent<PrefabLinkComponent>()) {
    auto &link = dstEntity.GetComponent<PrefabLinkComponent>();
    link.PrefabID = prefabID;
    link.LastKnownUpdate = prefabVersion;
  }

  auto &dstRel = dstEntity.GetComponent<RelationshipComponent>();
  auto &srcRel = srcPrefabEntity.GetComponent<RelationshipComponent>();

  // Match/create child entities in prefab child order while preserving existing child UUIDs/order where possible
  std::vector<UUID> updatedChildren;
  updatedChildren.reserve(srcRel.Children.size());

  std::vector<UUID> unmatchedExistingChildren = dstRel.Children;

  for (UUID srcChildUUID : srcRel.Children) {
    Entity srcChild = srcPrefabEntity.GetScene()->GetEntityByUUID(srcChildUUID);
    if (!srcChild)
      continue;

    const std::string &srcChildName = srcChild.GetComponent<TagComponent>().Tag;

    // Try matching an existing child by name first
    UUID matchedUUID = 0;
    for (auto it = unmatchedExistingChildren.begin(); it != unmatchedExistingChildren.end(); ++it) {
      Entity candidate = GetEntityByUUID(*it);
      if (candidate && candidate.GetComponent<TagComponent>().Tag == srcChildName) {
        matchedUUID = *it;
        unmatchedExistingChildren.erase(it);
        break;
      }
    }

    Entity dstChild;
    if (matchedUUID != 0) {
      dstChild = GetEntityByUUID(matchedUUID);
    } else {
      dstChild = CreateEntity(srcChildName, dstEntity.GetUUID());
    }

    if (dstChild) {
      dstChild.GetComponent<RelationshipComponent>().Parent = dstEntity.GetUUID();
      updatedChildren.push_back(dstChild.GetUUID());
      SyncPrefabHierarchyRecursive(dstChild, srcChild, prefabID, prefabVersion, forceUpdate);
    }
  }

  // Delete child entities that were removed from the prefab
  for (UUID leftoverChildUUID : unmatchedExistingChildren) {
    Entity leftoverChild = GetEntityByUUID(leftoverChildUUID);
    if (leftoverChild) {
      DeleteEntity(leftoverChild);
    }
  }

  dstRel.Children = std::move(updatedChildren);
}

void Scene::RecursiveUpdatePrefabs(const UUID &entityID, bool forceUpdate) {
  Entity entity = GetEntityByUUID(entityID);
  if (!entity)
    return;

  // Snapshot children before potential structural changes
  std::vector<UUID> childSnapshot = entity.GetComponent<RelationshipComponent>().Children;

  bool updatedThisNode = false;
  if (entity.HasComponent<PrefabLinkComponent>()) {
    auto &link = entity.GetComponent<PrefabLinkComponent>();
    Asset::MetaData metaData = Project::GetActive()->GetEditorAssetManager()->GetMetaData(link.PrefabID);

    if (metaData.LastUpdated > link.LastKnownUpdate || forceUpdate) {
      // Load/fetch the prefab's template scene & root entity for link.PrefabID
      Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(link.PrefabID); // adapt to your PrefabManager API
      Ref<Scene> prefabScene = prefab->GetScene();
      if (prefabScene) {
        UUID rootEntityID = prefabScene->m_RootEntities[0];
        Entity prefabRoot = prefabScene->GetEntityByUUID(rootEntityID); // adapt to your prefab root lookup
        if (prefabRoot) {
          SyncPrefabHierarchyRecursive(entity, prefabRoot, link.PrefabID, metaData.LastUpdated, forceUpdate);
          updatedThisNode = true;
        }
      }
    }
  }

  // If this node wasn't just synced from its own prefab root, recurse into existing children
  // (or still recurse if nested prefabs can have independent PrefabLinkComponents)
  if (!updatedThisNode) {
    for (const UUID &childID : childSnapshot) {
      RecursiveUpdatePrefabs(childID);
    }
  }
}

void Scene::UpdateAllPrefabs() {
  for (auto &entity : m_RootEntities) {
    RecursiveUpdatePrefabs(entity);
  }
}
Entity Scene::FindOwningCanvas(Entity entity) {
  // The step limit guards against a corrupted (cyclic) parent chain
  // turning into an infinite loop.
  size_t steps = 0;
  const size_t maxSteps = m_EntityMap.size();

  while (entity && steps++ <= maxSteps) {
    if (entity.HasComponent<CanvasComponent>())
      return entity;

    if (!entity.HasComponent<RelationshipComponent>())
      break;

    const UUID parentID = entity.GetComponent<RelationshipComponent>().Parent;
    if (parentID == 0)
      break;

    entity = GetEntityByUUID(parentID);
  }

  return {};
}

bool Scene::IsUnderCanvas(Entity entity) {
  return static_cast<bool>(FindOwningCanvas(entity));
}

const UIScreenRect *Scene::GetUIRect(UUID entityID) const {
  auto it = m_UIRects.find(entityID);
  return it != m_UIRects.end() ? &it->second : nullptr;
}
Entity Scene::PickUI(const glm::vec2 &p) {
    for (auto it = m_UIDrawList.rbegin(); it != m_UIDrawList.rend(); ++it) {
    const UIScreenRect *r = GetUIRect(it->Entity);
    if (!r || p.x < r->Min.x || p.x > r->Max.x || p.y < r->Min.y || p.y > r->Max.y)
      continue;
    Entity e = GetEntityByUUID(it->Entity);
    if (e && (e.HasComponent<PanelComponent>() || e.HasComponent<ButtonVisualComponent>()))
      return e;
  }
  return {};
}

Scene::UIElementInfo Scene::GetUIElementInfo(UUID id) const {
  UIElementInfo info;
  const UIDrawItem *item = FindUIDrawItem(id);
  const UIScreenRect *r = GetUIRect(id);
  if (!item || !r) return info;

  info.Valid = true;
  info.Min = r->Min; info.Max = r->Max;
  info.Hovered = m_UIInput.Hovered == id;
  info.Pressed = m_UIInput.Btn[0].Down && m_UIInput.Btn[0].PressTarget == id;
  info.Selected = m_UIInput.Selected == id;
  info.Alpha = item->Alpha;
  info.Interactable = item->Interactable;
  // (and the selectable's own Interactable flag, if present)
  return info;
}

void Scene::ExecuteSceneCommandQueue() {
  std::vector<std::function<void()>> copy;
  {
    std::scoped_lock<std::mutex> lock(m_SceneCommandQueueMutex);
    copy = m_SceneCommandQueue;
    m_SceneCommandQueue.clear();
  }

  for (auto &function : copy)
    function();

}


// On Component Added

template <typename T>
void Scene::OnComponentAdded(Entity entity, T &component) {
  static_assert(false, "Component Not Supported!");
}
template <>
void Scene::OnComponentAdded<LayerComponent>(Entity entity, LayerComponent &component) {}

template <>
void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent &component) {}

template <>
void Scene::OnComponentAdded<ObjectStateComponent>(Entity entity, ObjectStateComponent &component) {}

template <>
void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent &component) {
  if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
    component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <>
void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent &component) {}

template <>
void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent &component) {}

template <>
void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableRigidbody(entity, true);
}

template <>
void Scene::OnComponentAdded<NameComponent>(Entity entity, NameComponent &component) {}

template <>
void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableCollider(entity, true);
}

template <>
void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent &component) {
  if (!m_IsRunning)
    return;

  Physics2D::Get().EnableCollider(entity, true);
}

template <>
void Scene::OnComponentAdded<ScriptComponentContainer>(Entity entity, ScriptComponentContainer &component) {
  for (auto &sc : component.Scripts)
    sc.Loaded = false;

  if (!m_IsRunning)
    return;

  ScriptEngine::GetMutable().OnStartEntity(entity);
}

template <>
void Scene::OnComponentAdded<PrefabLinkComponent>(Entity entity, PrefabLinkComponent &component) {}

template <>
void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent &component) {}

template <>
void Scene::OnComponentAdded<RelationshipComponent>(Entity entity, RelationshipComponent &component) {}

template <>
void Scene::OnComponentAdded<RectTransformComponent>(Entity entity, RectTransformComponent &component) {}

template <>
void Scene::OnComponentAdded<ButtonVisualComponent>(Entity entity, ButtonVisualComponent &component) {}

template <>
void Scene::OnComponentAdded<PanelComponent>(Entity entity, PanelComponent &component) {}

template <>
void Scene::OnComponentAdded<CanvasComponent>(Entity entity, CanvasComponent &component) {}

template <>
void Scene::OnComponentAdded<CanvasGroupComponent>(Entity entity, CanvasGroupComponent &component) {}

template <>
void Scene::OnComponentAdded<SelectableComponent>(Entity entity, SelectableComponent &component) {}

template <>
void Scene::OnComponentAdded<EventListenerComponent>(Entity entity, EventListenerComponent &component) {}

} // namespace CatEngine
