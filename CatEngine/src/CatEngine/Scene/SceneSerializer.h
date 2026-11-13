#pragma once
#include "CatEngine/Core/Buffer.h"
#include "CatEngine/Scripting/ScriptEntityStorage.h"
#include "Scene.h"
#include "yaml-cpp/emitter.h"
namespace CatEngine {

class SceneSerializer {
public:
  SceneSerializer(const Ref<Scene> &scene);

  void SerializeEntityScriptVector(YAML::Emitter &out,const DataType &elementType, const void *elementPtr);
  void SerializeEntity(YAML::Emitter &out, Entity e);

  void Serialize(const std::string &filepath);
  void SerializeRuntime(const std::string &filepath);

  void DeserializeEntityScriptVector(YAML::Node &vectorNode, DataType elementType, size_t elementSize, VectorBuffer &outVectorData);
  void DeserializeEntity(YAML::Node &node, UUID parent);

  bool Deserialize(const std::string &filepath);
  bool DeserializeRuntime(const std::string &filepath);

private:
  Ref<Scene> m_Scene;
};
} // namespace CatEngine
