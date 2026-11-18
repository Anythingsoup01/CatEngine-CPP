#pragma once
#include "CatEngine/Scripting/ScriptEntityStorage.h"
#include "Entity.h"
#include "Prefab.h"
#include "yaml-cpp/emitter.h"

namespace CatEngine {
class PrefabSerializer {
public:
  PrefabSerializer(const Ref<Prefab> &prefab);


  void SerializeEntityScriptVector(YAML::Emitter &out, const DataType &elementType, const void *elementPtr);
  void SerializeEntity(YAML::Emitter &out, Entity e, Ref<Scene> rootScene = nullptr);

  void Serialize(const std::filesystem::path &filepath, Entity rootEntity, Ref<Scene> rootScene = nullptr);

  void DeserializeEntityScriptVector(YAML::Node &vectorNode, const DataType elementType, size_t elementSize, VectorBuffer &outVectorData);
  void DeserializeEntity(YAML::Node &node, UUID parent);

  bool Deserialize(const std::filesystem::path &filepath);

private:
  Ref<Prefab> m_Prefab;
};
} // namespace CatEngine
