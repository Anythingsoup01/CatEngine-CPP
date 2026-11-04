#pragma once

#include "CatEngine/AssetManager/Asset.h"

#include "Buffer.h"
#include "UniformBuffer.h"

#include "CatEngine/Serialization/StreamReader.h"
#include "CatEngine/Serialization/StreamWriter.h"

#include "CatEngine/Math/AABB.h"

#include <glm/glm.hpp>

namespace CatEngine {

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec3 Tangent;
  glm::vec3 Binormal;
  glm::vec2 Texcoord;
};

struct Index {
  uint32_t V1, V2, V3;
};

struct Triangle {
  Vertex V0, V1, V2;

  Triangle(const Vertex &v0, const Vertex &v1, const Vertex &v2)
      : V0(v0), V1(v1), V2(v2) {}
};

class Submesh {
public:
  uint32_t BaseVertex;
  uint32_t BaseIndex;
  uint32_t VertexCount;
  uint32_t IndexCount;

  glm::mat4 Transform{1.0f};
  glm::mat4 LocalTransform{1.0f};
  AABB BoundingBox;

  std::string NodeName, MeshName;

  static void Serialize(StreamWriter *serializer, const Submesh &instance) {
    serializer->WriteRaw(instance.BaseVertex);
    serializer->WriteRaw(instance.BaseIndex);
    serializer->WriteRaw(instance.VertexCount);
    serializer->WriteRaw(instance.IndexCount);
    serializer->WriteRaw(instance.Transform);
    serializer->WriteRaw(instance.LocalTransform);
    serializer->WriteRaw(instance.BoundingBox);
    serializer->WriteString(instance.NodeName);
    serializer->WriteString(instance.MeshName);
  }

  static void Deserialize(StreamReader *deserializer, Submesh &instance) {
    deserializer->ReadRaw(instance.BaseVertex);
    deserializer->ReadRaw(instance.BaseIndex);
    deserializer->ReadRaw(instance.VertexCount);
    deserializer->ReadRaw(instance.IndexCount);
    deserializer->ReadRaw(instance.Transform);
    deserializer->ReadRaw(instance.LocalTransform);
    deserializer->ReadRaw(instance.BoundingBox);
    deserializer->ReadString(instance.NodeName);
    deserializer->ReadString(instance.MeshName);
  }
};

struct MeshNode {
  uint32_t Parent = 0xffffffff;
  std::vector<uint32_t> Children;
  std::vector<uint32_t> Submeshes;

  std::string Name;
  glm::mat4 LocalTransform;

  inline bool IsRoot() const { return Parent == 0xffffffff; }

  static void Serialize(StreamWriter *serializer, const MeshNode &instance) {
    serializer->WriteRaw(instance.Parent);
    serializer->WriteArray(instance.Children);
    serializer->WriteArray(instance.Submeshes);
    serializer->WriteString(instance.Name);
    serializer->WriteRaw(instance.LocalTransform);
  }

  static void Deserialize(StreamReader *deserializer, MeshNode &instance) {
    deserializer->ReadRaw(instance.Parent);
    deserializer->ReadArray(instance.Children);
    deserializer->ReadArray(instance.Submeshes);
    deserializer->ReadString(instance.Name);
    deserializer->ReadRaw(instance.LocalTransform);
  }
};

class MeshSource : public Asset {
public:
  MeshSource() = default;
  MeshSource(const std::vector<Vertex> &vertices,
             const std::vector<Index> &indices, const glm::mat4 &transform);
  MeshSource(const std::vector<Vertex> &vertices,
             const std::vector<Index> &indices,
             const std::vector<Submesh> &submeshes);

  const std::vector<Triangle> GetTriangleCache(uint32_t index) {
    return m_TriangleCache[index];
  }

  Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
  Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

  const AABB &GetBoundingBox() const { return m_BoundingBox; }

  const MeshNode &GetRootNode() const { return m_Nodes[0]; }
  const std::vector<MeshNode> &GetNodes() const { return m_Nodes; }

  static AssetType GetStaticType() { return AssetType::MeshSource; }
  virtual AssetType GetType() const override { return GetStaticType(); }

private:
  std::vector<Submesh> m_Submeshes;

  Ref<VertexBuffer> m_VertexBuffer;
  Ref<IndexBuffer> m_IndexBuffer;

  std::vector<Vertex> m_Vertices;
  std::vector<Index> m_Indices;

  std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

  AABB m_BoundingBox;

  std::string m_FilePath;

  std::vector<MeshNode> m_Nodes;

  bool m_Runtime = false;

  friend class Scene;
  friend class Renderer;
  friend class OpenGLRenderAPI;
  friend class SceneHierarchyPanel;
  friend class Mesh;
};

// Dynamic Mesh

class Mesh : public Asset {
public:
  explicit Mesh(AssetHandle meshSource, bool generateColliders);
  Mesh(AssetHandle meshSource, const std::vector<uint32_t> &submeshes,
       bool generateColliders);
  virtual ~Mesh() = default;

  const std::vector<uint32_t> &GetSubmeshes() const { return m_Submeshes; }

  void SetSubmeshes(const std::vector<uint32_t> &submeshes,
                    Ref<MeshSource> meshSource);

  AssetHandle GetMeshSource() const { return m_MeshSource; }
  void SetMeshSource(AssetHandle meshSource) { m_MeshSource = meshSource; }

  bool ShouldGenerateColliders() const { return m_GenerateColliders; }

  static AssetType GetStaticType() { return AssetType::Mesh; }
  virtual AssetType GetType() const override { return GetStaticType(); }

private:
  AssetHandle m_MeshSource;
  std::vector<uint32_t> m_Submeshes;

  bool m_GenerateColliders = false;

  friend class Scene;
  friend class Renderer;
  friend class SceneHierarchyPanel;
};

} // namespace CatEngine
