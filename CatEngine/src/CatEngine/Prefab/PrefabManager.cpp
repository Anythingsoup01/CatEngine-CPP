#include "cepch.h" // IWYU pragma: keep
#include "PrefabManager.h"

#include "PrefabSerializer.h"

#include "CatEngine/AssetManager/AssetManager.h"
#include "CatEngine/Project/Project.h"
#include "CatEngine/Scene/Entity.h"

namespace CatEngine {

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

void PrefabManager::GetDirtyComponents(Entity entity, std::unordered_map<UUID, std::vector<std::string>> *changeMap) {
  if (!entity)
    return;

  std::vector<std::string> outDirty;
  if (entity.HasComponent<TransformComponent>() && entity.GetComponent<TransformComponent>().PrefabDirty)
    outDirty.push_back("Transform");
  if (entity.HasComponent<SpriteRendererComponent>() && entity.GetComponent<SpriteRendererComponent>().PrefabDirty)
    outDirty.push_back("Sprite Renderer");
  if (entity.HasComponent<CircleRendererComponent>() && entity.GetComponent<CircleRendererComponent>().PrefabDirty)
    outDirty.push_back("Circle Renderer");
  if (entity.HasComponent<CameraComponent>() && entity.GetComponent<CameraComponent>().PrefabDirty)
    outDirty.push_back("Camera");
  if (entity.HasComponent<Rigidbody2DComponent>() && entity.GetComponent<Rigidbody2DComponent>().PrefabDirty)
    outDirty.push_back("Rigidbody 2D");
  if (entity.HasComponent<BoxCollider2DComponent>() && entity.GetComponent<BoxCollider2DComponent>().PrefabDirty)
    outDirty.push_back("Box Collider 2D");
  if (entity.HasComponent<CircleCollider2DComponent>() && entity.GetComponent<CircleCollider2DComponent>().PrefabDirty)
    outDirty.push_back("Circle Collider 2D");
  if (entity.HasComponent<ScriptComponentContainer>()) {
    auto &scc = entity.GetComponent<ScriptComponentContainer>();
    if (scc.PrefabDirty) {
      for (auto &sc : scc.Scripts) {
        if (sc.PrefabDirty) {
          outDirty.push_back(sc.ClassName);
        }
      }
    }
  }
  if (entity.HasComponent<TagComponent>() && entity.GetComponent<TagComponent>().PrefabDirty)
    outDirty.push_back("Tag");
  if (entity.HasComponent<LayerComponent>() && entity.GetComponent<LayerComponent>().PrefabDirty)
    outDirty.push_back("Layer");
  if (entity.HasComponent<ObjectStateComponent>()) {
    auto &osc = entity.GetComponent<ObjectStateComponent>();
    if (osc.PrefabDirty)
      outDirty.push_back("Object State");
    if (osc.PrefabAdded)
      outDirty.push_back("New Component");

    for (auto &deletedComponent : osc.DeletedComponents) {
      std::string label = deletedComponent + " (Deleted)";
      outDirty.push_back(label);
    }
  }

  changeMap->emplace(entity.GetUUID(), outDirty);
  auto &rc = entity.GetComponent<RelationshipComponent>();
  for (auto &child : rc.Children) {
    Entity childEntity = entity.GetScene()->GetEntityByUUID(child);
    GetDirtyComponents(childEntity, changeMap);
  }
}

} // namespace CatEngine
