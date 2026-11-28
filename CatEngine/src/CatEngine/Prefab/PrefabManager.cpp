#include "cepch.h" // IWYU pragma: keep
#include "PrefabManager.h"

#include "PrefabSerializer.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine {

namespace PrefabUtil {
template <typename... Component>
static void CopyComponentIfExists(Entity dst, Entity src) {
  (
    [&]() {
      if (src.HasComponent<Component>())
        dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
    }(),
    ...);
}

template <typename... Component>
static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst,
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


using PrefabComponents = ComponentGroup<
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
    CanvasComponent,
    CanvasGroupComponent,
    PanelComponent,
    ButtonVisualComponent,
    SelectableComponent,
    EventListenerComponent>;

template <typename T>
static void AddIfDirty(Entity e, std::vector<std::string> &out, const char *label) {
  if (e.HasComponent<T>() && e.GetComponent<T>().PrefabDirty)
    out.push_back(label);
}

template <typename... Component>
static void ClearPrefabDirty(Entity e) {
  ([&]() {
    if (e.HasComponent<Component>())
      e.GetComponent<Component>().PrefabDirty = false;
  }(), ...);
}

template <typename... Component>
static void ClearPrefabDirty(ComponentGroup<Component...>, Entity e) {
  ClearPrefabDirty<Component...>(e);
}

// Removes components from dst that src no longer has.
template <typename... Component>
static void RemoveIfMissing(Entity dst, Entity src) {
  ([&]() {
    if (dst.HasComponent<Component>() && !src.HasComponent<Component>())
      dst.RemoveComponent<Component>();
  }(), ...);
}

template <typename... Component>
static void RemoveIfMissing(ComponentGroup<Component...>, Entity dst, Entity src) {
  RemoveIfMissing<Component...>(dst, src);
}
}

using namespace PrefabUtil;

void PrefabManager::CreateNewPrefab(const std::filesystem::path &targetDirectory) {
  std::filesystem::path prefabPath =  targetDirectory / "NewPrefab.catprefab";

  int counter = 0;
  while (std::filesystem::exists(prefabPath)) {
    prefabPath =
      targetDirectory / (prefabPath.stem().generic_string() + "_" + std::to_string(++counter) + prefabPath.extension().generic_string());
  }

  Ref<Prefab> newPrefab = CreateRef<Prefab>();
  auto scene = newPrefab->GetScene();
  scene->CreateEntity("New Entity");
  PrefabSerializer serializer(newPrefab);
  serializer.Serialize(prefabPath);

  std::filesystem::path relativePath = std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
  Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(relativePath);

  AddPrefab(newPrefab->GetAssetHandle());
}

void PrefabManager::CreateNewPrefabFromEntity(Entity e, const std::filesystem::path &targetDirectory) {
  if (!e)
    return;

  std::string entityName = e.GetName();
  std::filesystem::path prefabPath = targetDirectory / (entityName + ".catprefab");

  int counter = 0;
  while (std::filesystem::exists(prefabPath)) {
    prefabPath =
      targetDirectory /
      (entityName + "_" + std::to_string(++counter) + ".catprefab");
  }

  Ref<Prefab> newPrefab = CreateRef<Prefab>();
  PrefabSerializer serializer(newPrefab);
  serializer.Serialize(prefabPath, e);

  std::filesystem::path relativePath = std::filesystem::relative(prefabPath, Project::GetAssetDirectory());
  Project::GetActive()->GetEditorAssetManager()->ImportNewAsset(relativePath);

  AddPrefab(newPrefab->GetAssetHandle());
}

void PrefabManager::AddPrefab(const AssetHandle &handle) {
  m_Prefabs.push_back(handle);
}

void PrefabManager::RemovePrefab(const AssetHandle &handle) {
  for (auto it = m_Prefabs.begin(); it != m_Prefabs.end(); it++) {
    if (handle == *it) {
      m_Prefabs.erase(it);
      break;
    }
  }
}

std::unordered_map<UUID, std::vector<std::string>> PrefabManager::GetDirtyEntities(const AssetHandle &prefabID, Entity rootEntity) {
  if (!AssetManager::IsAssetHandleValid(prefabID))
    return {};

  std::unordered_map<UUID, std::vector<std::string>> out;
  Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);

  GetDirtyComponents(rootEntity, &out);

  out[1] = std::vector<std::string>();
  auto &plc = rootEntity.GetComponent<PrefabLinkComponent>();
  for (auto &deletedEntity : plc.DeletedEntities) {
    std::string label = deletedEntity + " (Deleted)";
    out[1].push_back(label);
  }

  return out;
}

void PrefabManager::ApplyOverridesToPrefab(const AssetHandle &prefabID, Entity rootSceneEntity) {
  if (!AssetManager::IsAssetHandleValid(prefabID) || !rootSceneEntity || !rootSceneEntity.GetScene())
    return;

  Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabID);
  if (!prefab)
    return;

  Ref<Scene> prefabScene = prefab->GetScene();
  if (!prefabScene || prefabScene->m_RootEntities.empty())
    return;

  Entity prefabRoot = prefabScene->GetEntityByUUID(prefabScene->m_RootEntities[0]);
  if (!prefabRoot)
    return;

  SyncPrefabEntityFromSceneRecursive(prefabRoot, rootSceneEntity, true);
  ClearInstanceOverrideFlagsRecursive(rootSceneEntity);

  // Save updated prefab asset & bump LastUpdated metadata so other instances don't re-pull stale state
  auto assetManager = Project::GetActive()->GetEditorAssetManager();
  Asset::MetaData &metaData = assetManager->GetMetaData(prefabID);
  PrefabSerializer serializer(prefab);
  serializer.Serialize(metaData.SourceFilePath);

  Project::GetActive()->GetEditorAssetManager()->SerializeAsset(prefabID);

  if (rootSceneEntity.HasComponent<PrefabLinkComponent>()) {
    Asset::MetaData &metaData = assetManager->GetMetaData(prefabID);
    rootSceneEntity.GetComponent<PrefabLinkComponent>().LastKnownUpdate = metaData.LastUpdated;
  }
}

void PrefabManager::GetDirtyComponents(Entity entity, std::unordered_map<UUID, std::vector<std::string>> *changeMap) {
  if (!entity)
    return;

  std::vector<std::string> outDirty;

  AddIfDirty<TransformComponent>(entity, outDirty, "Transform");
  AddIfDirty<RectTransformComponent>(entity, outDirty, "Rect Transform");
  AddIfDirty<CanvasComponent>(entity, outDirty, "Canvas");
  AddIfDirty<CanvasGroupComponent>(entity, outDirty, "Canvas Group");
  AddIfDirty<PanelComponent>(entity, outDirty, "Panel");
  AddIfDirty<ButtonVisualComponent>(entity, outDirty, "Button");
  AddIfDirty<SelectableComponent>(entity, outDirty, "Selectable");
  AddIfDirty<EventListenerComponent>(entity, outDirty, "Event Listener");
  AddIfDirty<SpriteRendererComponent>(entity, outDirty, "Sprite Renderer");
  AddIfDirty<CircleRendererComponent>(entity, outDirty, "Circle Renderer");
  AddIfDirty<CameraComponent>(entity, outDirty, "Camera");
  AddIfDirty<Rigidbody2DComponent>(entity, outDirty, "Rigidbody 2D");
  AddIfDirty<BoxCollider2DComponent>(entity, outDirty, "Box Collider 2D");
  AddIfDirty<CircleCollider2DComponent>(entity, outDirty, "Circle Collider 2D");
  AddIfDirty<TagComponent>(entity, outDirty, "Tag");
  AddIfDirty<LayerComponent>(entity, outDirty, "Layer");

  // Scripts report per script, not per container
  if (entity.HasComponent<ScriptComponentContainer>()) {
    auto &scc = entity.GetComponent<ScriptComponentContainer>();
    if (scc.PrefabDirty) {
      for (auto &sc : scc.Scripts)
        if (sc.PrefabDirty)
          outDirty.push_back(sc.ClassName);
    }
  }

  if (entity.HasComponent<ObjectStateComponent>()) {
    auto &osc = entity.GetComponent<ObjectStateComponent>();
    if (osc.PrefabDirty)
      outDirty.push_back("Object State");
    if (osc.PrefabAdded)
      outDirty.push_back("New Component");

    for (auto &deletedComponent : osc.DeletedComponents)
      outDirty.push_back(deletedComponent + " (Deleted)");
  }

  changeMap->emplace(entity.GetUUID(), outDirty);
  auto &rc = entity.GetComponent<RelationshipComponent>();
  for (auto &child : rc.Children) {
    Entity childEntity = entity.GetScene()->GetEntityByUUID(child);
    GetDirtyComponents(childEntity, changeMap);
  }
}


void PrefabManager::CopyComponentsToPrefabEntity(Entity dstPrefabEntity, Entity srcSceneEntity, bool isRoot) {
  if (!dstPrefabEntity || !srcSceneEntity)
    return;

  TransformComponent storedTC = dstPrefabEntity.GetComponent<TransformComponent>();

  CopyComponentIfExists(PrefabComponents{}, dstPrefabEntity, srcSceneEntity);

  if (isRoot) {
    dstPrefabEntity.GetComponent<TransformComponent>() = storedTC;
  }

  if (dstPrefabEntity.HasComponent<ScriptComponentContainer>()) {
    ScriptEngine::GetMutable().CopyScriptFields(srcSceneEntity, dstPrefabEntity);
  }

  // Remove components that exist on the prefab entity but were removed from the scene entity
  RemoveIfMissing(PrefabComponents{}, dstPrefabEntity, srcSceneEntity);}

void PrefabManager::SyncPrefabEntityFromSceneRecursive(Entity dstPrefabEntity, Entity srcSceneEntity, bool isRoot) {
  if (!dstPrefabEntity || !srcSceneEntity)
    return;

  CopyComponentsToPrefabEntity(dstPrefabEntity, srcSceneEntity, isRoot);

  Scene *prefabScene = dstPrefabEntity.GetScene();
  auto &dstRel = dstPrefabEntity.GetComponent<RelationshipComponent>();
  auto &srcRel = srcSceneEntity.GetComponent<RelationshipComponent>();

  std::vector<UUID> updatedPrefabChildren;
  updatedPrefabChildren.reserve(srcRel.Children.size());

  std::vector<UUID> unmatchedPrefabChildren = dstRel.Children;

  for (UUID srcChildUUID : srcRel.Children) {
    Entity srcChild = srcSceneEntity.GetScene()->GetEntityByUUID(srcChildUUID);
    if (!srcChild)
      continue;

    const std::string &srcChildName = srcChild.GetComponent<TagComponent>().Tag;

    // Match an existing prefab child by name first
    UUID matchedPrefabChildUUID = 0;
    for (auto it = unmatchedPrefabChildren.begin(); it != unmatchedPrefabChildren.end(); ++it) {
      Entity candidate = prefabScene->GetEntityByUUID(*it);
      if (candidate && candidate.GetComponent<TagComponent>().Tag == srcChildName) {
        matchedPrefabChildUUID = *it;
        unmatchedPrefabChildren.erase(it);
        break;
      }
    }

    Entity dstChild;
    if (matchedPrefabChildUUID != 0) {
      dstChild = prefabScene->GetEntityByUUID(matchedPrefabChildUUID);
    } else {
      dstChild = prefabScene->CreateEntity(srcChildName, dstPrefabEntity.GetUUID());
    }

    if (dstChild) {
      dstChild.GetComponent<RelationshipComponent>().Parent = dstPrefabEntity.GetUUID();
      updatedPrefabChildren.push_back(dstChild.GetUUID());
      SyncPrefabEntityFromSceneRecursive(dstChild, srcChild, false);
    }
  }

  // Delete prefab child entities that were removed from the scene hierarchy
  for (UUID leftoverChildUUID : unmatchedPrefabChildren) {
    Entity leftoverChild = prefabScene->GetEntityByUUID(leftoverChildUUID);
    if (leftoverChild) {
      prefabScene->DeleteEntity(leftoverChild);
    }
  }

  dstRel.Children = std::move(updatedPrefabChildren);
}

void PrefabManager::ClearInstanceOverrideFlagsRecursive(Entity current) {
  if (!current)
    return;

  ClearPrefabDirty<NameComponent>(current);
  ClearPrefabDirty(PrefabComponents{}, current);

  if (current.HasComponent<PrefabLinkComponent>()) {
    auto &plc = current.GetComponent<PrefabLinkComponent>();
    plc.PrefabDirty = false;
    plc.DeletedEntities.clear();
  }

  if (current.HasComponent<ObjectStateComponent>()) {
    auto &osc = current.GetComponent<ObjectStateComponent>();
    osc.PrefabAdded = false;
    osc.DeletedComponents.clear();
  }

  if (current.HasComponent<ScriptComponentContainer>()) {
    for (auto &sc : current.GetComponent<ScriptComponentContainer>().Scripts)
      sc.PrefabDirty = false;
  }

  if (current.HasComponent<RelationshipComponent>()) {
    std::vector<UUID> childSnapshot = current.GetComponent<RelationshipComponent>().Children;
    for (UUID childUUID : childSnapshot) {
      Entity child = current.GetScene()->GetEntityByUUID(childUUID);
      if (child) {
        ClearInstanceOverrideFlagsRecursive(child);
      }
    }
  }
}

} // namespace CatEngine
