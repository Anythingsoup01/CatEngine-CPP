#pragma once
#include "Scene.h"
#include "yaml-cpp/emitter.h"
namespace CatEngine {

class SceneSerializer {
public:
  SceneSerializer(const Ref<Scene> &scene);

  void SerializeEntity(YAML::Emitter &out, Entity e);

  void Serialize(const std::string &filepath);
  void SerializeRuntime(const std::string &filepath);

  void DeserializeEntity(YAML::Node &node, UUID parent);

  bool Deserialize(const std::string &filepath);
  bool DeserializeRuntime(const std::string &filepath);

private:
  Ref<Scene> m_Scene;
};
} // namespace CatEngine
