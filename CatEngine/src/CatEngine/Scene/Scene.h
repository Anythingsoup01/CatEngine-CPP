#pragma once

#include "Components/UI/UITypes.h"

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/TimeStep.h"
#include "CatEngine/Core/UUID.h"
#include "CatEngine/Renderer/EditorCamera.h"


#include <entt.hpp>
#include <vector>


class b2World;

namespace CatEngine {

class Entity;

class Scene : public Asset {
public:
  Scene();
  ~Scene();

  void CopyTo(Ref<Scene> &target);

  Entity CreateEntity(const std::string &name = std::string(), UUID parent = 0);
  Entity CreateEntityWithUUID(UUID uuid, const std::string &name = std::string(), UUID parent = 0);
  Entity CreateCanvas(const std::string &name = std::string(), UUID parent = 0);
  Entity CreatePanel(UUID parent = 0);
  Entity CreateButton(UUID parent = 0);

  void DeleteEntity(Entity entity);

  void SetEntityLocked(Entity entity, bool locked);
  void SetEntityChildLockedRecursive(Entity child, bool parentLocked);

  void SetEntityActive(Entity entity, bool active);
  void SetEntityChildActiveRecursive(Entity child, bool parentActive);

  Entity InstantiatePrefab(const UUID &prefabID, const UUID &parentID = 0);

  void DrawEntityRecursive(Entity e, const glm::mat4 &parentWorldMatrix);

  void OnPauseStart();
  void OnPauseStop();

  void OnRuntimeStart();
  void OnRuntimeStop();

  void OnSimulationStart();
  void OnSimulationStop();

  void OnUpdateEditor(Time ts, EditorCamera &camera);
  void OnUpdateMainCameraPreview(Time ts, const glm::vec2 &viewSize = {0, 0});
  void OnUpdateSimulation(Time time, EditorCamera &camera);
  void OnUpdateRuntime(Time ts, const glm::vec2 &viewSize = {0, 0});

  void ShutdownRuntime();

  void OnViewportResize(uint32_t width, uint32_t height);

  void DuplicateEntity(Entity entity);
  Entity PasteEntity(Entity entity);

  Entity FindEntityByName(const std::string_view &name);
  Entity GetEntityByUUID(UUID entityID);

  bool ComputeCanvasToWorld(glm::mat4 &out);
  float GetUIScale(const UUID &id) const;

  void SetUIInput(const UIInputFrame &frame);   // call every frame in play mode

  bool IsPointerOverUI() const { return m_UIInput.RawHit != 0; } // scripts: ignore world clicks
  UUID GetHoveredUI() const { return m_UIInput.Hovered; }
  UUID GetSelectedUI() const { return m_UIInput.Selected; }
  void SetSelectedUI(Entity e);                 // pass {} to clear

  Entity GetPrimaryCameraEntity();

  template <typename... Components> auto GetAllComponentsWith() {
    return m_Registry.view<Components...>();
  }

  entt::registry &GetReg() { return m_Registry; }

  bool IsRunning() { return m_IsRunning; }

  void ReparentEntity(Entity e, Entity newParent);
  void MoveEntityBefore(Entity e, Entity targetEntity);
  void MoveEntityAfter(Entity e, Entity targetEntity);

  void RegisterParents();

  void UpdateAllPrefabs();

  Entity FindOwningCanvas(Entity entity);
  bool IsUnderCanvas(Entity entity);

  std::vector<UUID> &GetRootEntities() { return m_RootEntities; }

  const UIScreenRect *GetUIRect(UUID entityID) const;
  Entity PickUI(const glm::vec2 &canvasPoint); // editor picking: topmost visible graphic

  static AssetType GetStaticType() { return AssetType::Scene; }
  virtual AssetType GetType() const override { return GetStaticType(); }

public:
  struct UIElementInfo {
    bool Valid = false;
    glm::vec2 Min{0.0f}, Max{0.0f};     // viewport pixels, Y-up
    bool Hovered = false, Pressed = false, Selected = false;
    bool Interactable = false;          // CanvasGroup chain AND selectable flag
    float Alpha = 1.0f;                 // accumulated CanvasGroup alpha
  };

  UIElementInfo GetUIElementInfo(UUID id) const;

private:
  template <typename T> void OnComponentAdded(Entity entity, T &component);

private:

  struct UIButtonState {
    bool Down = false, Ignored = false, Dragging = false;
    UUID PressTarget = 0, ClickTarget = 0, DragTarget = 0;
    glm::vec2 PressPos{0.0f}, LastPos{0.0f};
    UUID LastClickTarget = 0;
    float LastClickTime = -1.0f;
    glm::vec2 LastClickPos{0.0f};
    int ClickCount = 0;
  };

  struct UIInputState {
    UIInputFrame Frame;
    glm::vec2 LastPointerPos{0.0f};
    UUID RawHit = 0, Hovered = 0, Selected = 0;
    std::array<UIButtonState, 3> Btn;
    float Time = 0.0f;
  } m_UIInput;

private:
  void RecursiveCopyTo(Ref<Scene> &target, const UUID &entityID);

  void SyncEntityComponentsFromPrefab(Entity dstEntity, Entity srcPrefabEntity, bool preserveInstanceTransform, bool forceUpdate);
  void SyncPrefabHierarchyRecursive(Entity dstEntity, Entity srcPrefabEntity, AssetHandle prefabID, uint64_t prefabVersion, bool forceUpdate);
  void RecursiveUpdatePrefabs(const UUID &entityID, bool forceUpdate = false);

  void OnPhysics2DStart();
  void OnPhysics2DUpdate(Time ts);
  void OnPhysics2DStop();

  void OnScriptStart();

  void UpdateTransforms();
  void CalculateTransformRecursive(Entity e, const glm::mat4 &parentWorldMatrix);

  void UpdateUILayout(const glm::vec2 &sizeOverride = {0, 0});
  void UpdateUINodeRecursive(Entity entity, const UIScreenRect &parentRect, float canvasScale,
                           const UIGroupState &parentGroup, UUID inheritedTint,
                           std::vector<UIDrawItem> &out);

  void UpdateUIInput(float dt);
  void UpdateUITints(float dt);
  void ValidateUIInputState();
  const UIDrawItem *FindUIDrawItem(UUID id) const;
  UUID RaycastUI(const glm::vec2 &p);
  UUID ResolveUITarget(UUID raw, uint32_t mask, bool needSelectable = false);
  bool SendUIEvent(UIEventType type, UUID target, int button = 0,
                   glm::vec2 delta = {0.0f, 0.0f}, int clickCount = 0,
                   UINavDirection dir = UINavDirection::None);
  void SetUIHovered(UUID next);
  void ProcessUIPointerButton(int b, bool down, bool pointerValid, const glm::vec2 &pos);
  void ProcessUINavigation(const UIInputFrame &frame);
  std::vector<Entity> CollectNavigableUI();
  Entity FindNavigationTarget(Entity from, UINavDirection dir);
  Entity FindTabTarget(Entity from, bool forward);
  glm::vec4 UITintFor(const UIDrawItem &item) const;
  void DrawUIList(const glm::mat4 &toWorld);

  void DrawUICanvases(EditorCamera &camera);
  void DrawUICanvases();


  void DrawUIQuad(const UIScreenRect &rect, const glm::vec4 &color, AssetHandle texture, int entityID, const glm::mat4 &canvasToWorld);

  Entity FindOrCreateCanvas();
  Entity CreateUIElement(const std::string &name, Entity parent);

  // Queues the entity in a vector to be deleted
  // after disabling any physics or scripts
  void DeleteRuntimeEntity(Entity e);

  // Deletes the entities in the queue after
  // having physics and scripts disabled
  void DeleteRuntimeEntities();

  // Deletes the edit time entity
  void DeleteEditTimeEntity(Entity e);

private:
  void ExecuteSceneCommandQueue();
private:
  entt::registry m_Registry;
  uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;


  float m_FixedTimeStep = 0.02f;
  float m_Accumulator = 0.0f;

  std::vector<UUID> m_DeletedEntities;
  std::unordered_map<UUID, entt::entity> m_EntityMap;
  std::vector<UUID> m_RootEntities;

  std::unordered_map<UUID, UIScreenRect> m_UIRects;
  std::vector<UUID> m_UICanvasOrder;

  float m_UIDragThreshold = 8.0f;     // pixels before a press becomes a drag
  float m_UIDoubleClickTime = 0.3f;   // seconds
  std::vector<UIDrawItem> m_UIDrawList;                 // back-to-front
  std::unordered_map<UUID, uint32_t> m_UIDrawIndex;     // UUID -> index into m_UIDrawList
  std::unordered_map<UUID, glm::vec4> m_UITints;        // current faded tint per selectable

  bool m_UIVirtualCanvas = false;            // editor only: lay out without a game camera
  glm::vec2 m_UILayoutSize{1920.0f, 1080.0f}; // size used by the most recent layout pass
  static constexpr float kVirtualPixelsPerUnit = 200.0f; // 1920x1080 -> 9.6 x 5.4 world units
  glm::vec2 m_UIViewOrigin{0.0f};

  bool m_IsDirty = false;

  bool m_IsRunning = false;
  bool m_IsPaused = false;

  std::vector<std::function<void()>> m_SceneCommandQueue;
  std::mutex m_SceneCommandQueueMutex;

  friend class Entity;
  friend class Physics2D;
  friend class PrefabManager;
  friend class InspectorPanel;
};
} // namespace CatEngine
