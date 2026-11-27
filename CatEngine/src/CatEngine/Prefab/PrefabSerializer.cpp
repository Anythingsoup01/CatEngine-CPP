#include "cepch.h" // IWYU pragma: keep

#include "PrefabSerializer.h"

#include "CatEngine/Core/Formatter.h"
#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine {

PrefabSerializer::PrefabSerializer(const Ref<Prefab> &prefab)
    : m_Prefab(prefab) {}

void PrefabSerializer::SerializeEntityScriptVector(YAML::Emitter &out,
                                             const DataType &elementType,
                                             const void *elementPtr) {
  if (!elementPtr)
    return;

  switch (elementType) {
  case DataType::Short:
    out << *reinterpret_cast<const int16_t *>(elementPtr);
    break;
  case DataType::UShort:
    out << *reinterpret_cast<const uint16_t *>(elementPtr);
    break;
  case DataType::Int:
    out << *reinterpret_cast<const int32_t *>(elementPtr);
    break;
  case DataType::UInt:
    out << *reinterpret_cast<const uint32_t *>(elementPtr);
    break;
  case DataType::Long:
    out << *reinterpret_cast<const int64_t *>(elementPtr);
    break;
  case DataType::ULong:
    out << *reinterpret_cast<const uint64_t *>(elementPtr);
    break;
  case DataType::Float:
    out << *reinterpret_cast<const float *>(elementPtr);
    break;
  case DataType::Double:
    out << *reinterpret_cast<const double *>(elementPtr);
    break;
  case DataType::Bool:
    out << *reinterpret_cast<const bool *>(elementPtr);
    break;
  case DataType::Vector2:
    out << *reinterpret_cast<const glm::vec2 *>(elementPtr);
    break;
  case DataType::Vector3:
    out << *reinterpret_cast<const glm::vec3 *>(elementPtr);
    break;
  case DataType::Vector4:
    out << *reinterpret_cast<const glm::vec4 *>(elementPtr);
    break;
  case DataType::Entity:
  case DataType::Texture2D:
  case DataType::Prefab:
  case DataType::ScriptComponent:
    out << *reinterpret_cast<const uint64_t *>(elementPtr);
    break;
  default:
    break;
  }
}
void PrefabSerializer::SerializeEntity(YAML::Emitter &out, Entity entity) {
  CE_PROFILE_FUNCTION();
  out << YAML::BeginMap; // Entity
  out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IDComponent>().ID;
  if (entity.HasComponent<TagComponent>()) {
    auto &tc = entity.GetComponent<TagComponent>();
    out << YAML::Key << "TagComponent";
    out << YAML::BeginMap; // TagComponent

    out << YAML::Key << "Tag" << YAML::Value << tc.Tag;
    out << YAML::Key << "PrefabDirty" << YAML::Value << tc.PrefabDirty;

    tc.EditDirty = false;
    out << YAML::EndMap; // TagComponent
  }
  if (entity.HasComponent<LayerComponent>()) {
    auto &lc = entity.GetComponent<LayerComponent>();
    out << YAML::Key << "LayerComponent";
    out << YAML::BeginMap; // LayerComponent

    out << YAML::Key << "Layer" << YAML::Value << lc.Layer;
    out << YAML::Key << "PrefabDirty" << YAML::Value << lc.PrefabDirty;

    lc.EditDirty = false;
    out << YAML::EndMap; // LayerComponent
  }
  if (entity.HasComponent<NameComponent>()) {
    auto &nc = entity.GetComponent<NameComponent>();
    out << YAML::Key << "NameComponent";
    out << YAML::BeginMap; // NameComponent

    out << YAML::Key << "Name" << YAML::Value << nc.Name;
    out << YAML::Key << "PrefabDirty" << YAML::Value << nc.PrefabDirty;

    nc.EditDirty = false;
    out << YAML::EndMap; // NameComponent
  }
  if (entity.HasComponent<ObjectStateComponent>()) {
    auto &osc = entity.GetComponent<ObjectStateComponent>();
    out << YAML::Key << "ObjectStateComponent";
    out << YAML::BeginMap; // ObjectStateComponent

    out << YAML::Key << "Locked" << YAML::Value << osc.Locked;
    out << YAML::Key << "SelfLocked" << YAML::Value << osc.SelfLocked;
    out << YAML::Key << "HierarchyLocked" << YAML::Value << osc.HierarchyLocked;

    out << YAML::Key << "Active" << YAML::Value << osc.Active;
    out << YAML::Key << "SelfActive" << YAML::Value << osc.SelfActive;
    out << YAML::Key << "HierarchyActive" << YAML::Value << osc.HierarchyActive;

    out << YAML::Key << "PrefabDirty" << YAML::Value << osc.PrefabDirty;
    out << YAML::Key << "PrefabAdded" << YAML::Value << osc.PrefabAdded;

    osc.EditDirty = false;
    out << YAML::EndMap; // ObjectStateComponent
  }
  if (entity.HasComponent<PrefabLinkComponent>()) {
    auto &plc = entity.GetComponent<PrefabLinkComponent>();
    out << YAML::Key << "PrefabLinkComponent";
    out << YAML::BeginMap; // PrefabLinkComponent

    out << YAML::Key << "PrefabID" << YAML::Value << plc.PrefabID;
    out << YAML::Key << "LastKnownUpdate" << YAML::Value << plc.LastKnownUpdate;
    out << YAML::Key << "PrefabDirty" << YAML::Value << plc.PrefabDirty;

    out << YAML::EndMap; // PrefabLinkComponent
  }
  if (entity.HasComponent<TransformComponent>()) {
    auto &tc = entity.GetComponent<TransformComponent>();
    out << YAML::Key << "TransformComponent";
    out << YAML::BeginMap; // TransformComponent

    out << YAML::Key << "LocalPosition" << YAML::Value << tc.LocalPosition;
    out << YAML::Key << "LocalRotation" << YAML::Value << tc.LocalRotation;
    out << YAML::Key << "LocalScale" << YAML::Value << tc.LocalScale;
    out << YAML::Key << "PrefabDirty" << YAML::Value << tc.PrefabDirty;

    tc.EditDirty = false;
    out << YAML::EndMap; // TransformComponent
  }

  if (entity.HasComponent<SpriteRendererComponent>()) {
    auto &src = entity.GetComponent<SpriteRendererComponent>();
    out << YAML::Key << "SpriteRendererComponent";
    out << YAML::BeginMap; // SpriteRendererComponent

    out << YAML::Key << "Color" << YAML::Value << src.Color;
    out << YAML::Key << "Texture" << YAML::Value << src.Texture;
    out << YAML::Key << "Tiling" << YAML::Value << src.TilingFactor;
    out << YAML::Key << "PrefabDirty" << YAML::Value << src.PrefabDirty;

    src.EditDirty = false;
    out << YAML::EndMap; // SpriteRendererComponent
  }

  if (entity.HasComponent<CircleRendererComponent>()) {
    auto &crc = entity.GetComponent<CircleRendererComponent>();
    out << YAML::Key << "CircleRendererComponent";
    out << YAML::BeginMap; // CircleRendererComponent

    out << YAML::Key << "Color" << YAML::Value << crc.Color;
    out << YAML::Key << "Thickness" << YAML::Value << crc.Thickness;
    out << YAML::Key << "Fade" << YAML::Value << crc.Fade;
    out << YAML::Key << "PrefabDirty" << YAML::Value << crc.PrefabDirty;

    crc.EditDirty = false;
    out << YAML::EndMap; // CircleRendererComponent
  }

  if (entity.HasComponent<CameraComponent>()) {
    out << YAML::Key << "CameraComponent";
    out << YAML::BeginMap; // CameraComponent

    auto &cameraComponent = entity.GetComponent<CameraComponent>();
    auto &camera = cameraComponent.Camera;

    out << YAML::Key << "Camera" << YAML::Value;
    out << YAML::BeginMap; // Camera
    out << YAML::Key << "ProjectionType" << YAML::Value
        << (int)camera.GetProjectionType();
    out << YAML::Key << "PerspectiveFOV" << YAML::Value
        << camera.GetPerspectiveVerticalFov();
    out << YAML::Key << "PerspectiveNear" << YAML::Value
        << camera.GetPerspectiveNearClip();
    out << YAML::Key << "PerspectiveFar" << YAML::Value
        << camera.GetPerspectiveFarClip();
    out << YAML::Key << "OrthographicSize" << YAML::Value
        << camera.GetOrthographicSize();
    out << YAML::Key << "OrthographicNear" << YAML::Value
        << camera.GetOrthographicNearClip();
    out << YAML::Key << "OrthographicFar" << YAML::Value
        << camera.GetOrthographicFarClip();
    out << YAML::EndMap; // Camera

    out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
    out << YAML::Key << "FixedAspectRatio" << YAML::Value
        << cameraComponent.FixedAspectRatio;
    out << YAML::Key << "PrefabDirty" << YAML::Value << cameraComponent.PrefabDirty;

    cameraComponent.EditDirty = false;
    out << YAML::EndMap; // CameraComponent
  }

  if (entity.HasComponent<Rigidbody2DComponent>()) {
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

    out << YAML::Key << "Rigidbody2DComponent";
    out << YAML::BeginMap; // Rigidbody2DComponent
    out << YAML::Key << "BodyType" << YAML::Value << (int)rb2d.Type;
    out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
    out << YAML::Key << "PrefabDirty" << YAML::Value << rb2d.PrefabDirty;
    rb2d.EditDirty = false;
    out << YAML::EndMap; // Rigidbody2DComponent
  }

  if (entity.HasComponent<BoxCollider2DComponent>()) {
    auto &bc2d = entity.GetComponent<BoxCollider2DComponent>();
    out << YAML::Key << "BoxCollider2DComponent" << YAML::Value;
    out << YAML::BeginMap; // BoxCollider2DComponent
    out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
    out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
    out << YAML::Key << "Rotation" << YAML::Value << bc2d.Rotation;
    out << YAML::Key << "ShowColliderBounds" << YAML::Value << bc2d.ShowColliderBounds;
    out << YAML::Key << "IsTrigger" << YAML::Value << bc2d.IsTrigger;
    out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
    out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
    out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
    out << YAML::Key << "RestitutionThreshold" << YAML::Value
        << bc2d.RestitutionThreshold;
    out << YAML::Key << "PrefabDirty" << YAML::Value << bc2d.PrefabDirty;
    bc2d.EditDirty = false;
    out << YAML::EndMap; // BoxCollider2DComponent
  }

  if (entity.HasComponent<CircleCollider2DComponent>()) {
    auto &cc2d = entity.GetComponent<CircleCollider2DComponent>();
    out << YAML::Key << "CircleCollider2DComponent" << YAML::Value;
    out << YAML::BeginMap; // CircleCollider2DComponent
    out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
    out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
    out << YAML::Key << "ShowColliderBounds" << YAML::Value << cc2d.ShowColliderBounds;
    out << YAML::Key << "IsTrigger" << YAML::Value << cc2d.IsTrigger;
    out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
    out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
    out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
    out << YAML::Key << "RestitutionThreshold" << YAML::Value
        << cc2d.RestitutionThreshold;
    out << YAML::Key << "PrefabDirty" << YAML::Value << cc2d.PrefabDirty;
    cc2d.EditDirty = false;
    out << YAML::EndMap; // CircleCollider2DComponent
  }

  if (entity.HasComponent<ScriptComponentContainer>()) {
    auto &scriptEngine = ScriptEngine::GetMutable();
    auto &container = entity.GetComponent<ScriptComponentContainer>();
    out << YAML::Key << "ScriptComponentContainer" << YAML::Value;
    out << YAML::BeginSeq; // ScriptComponentContainer Start
    for (auto &sc : container.Scripts) {
      out << YAML::BeginMap; // ScriptComponent Start
      out << YAML::Key << "ScriptComponent" << YAML::Value;
      out << YAML::BeginMap; // ScriptComponent Start
      out << YAML::Key << "ScriptID" << YAML::Value << sc.ScriptID;
      out << YAML::Key << "ClassName" << YAML::Value << sc.ClassName;
      out << YAML::Key << "PrefabDirty" << YAML::Value << sc.PrefabDirty;
      out << YAML::Key << "Fields" << YAML::Value;
      out << YAML::BeginSeq; // Fields Start
      const FieldMap &fieldMap =
          scriptEngine.GetInitializedFields(entity.GetUUID(), sc.ScriptID);
      for (auto &[fieldID, field] : fieldMap) {
        out << YAML::BeginMap; // Field Map Start
        out << YAML::Key << "FieldID" << YAML::Value << fieldID;
        out << YAML::Key << "FieldName" << YAML::Value << field.Field.Name;
        std::string fieldTypeStr = DataTypeToString(field.Field.Type);
        if (fieldTypeStr.empty()) {
          fieldTypeStr = field.Field.TypeName;
        }
        out << YAML::Key << "FieldType" << YAML::Value << fieldTypeStr;
        out << YAML::Key << "FieldSize" << YAML::Value << field.Field.Size;
        out << YAML::Key << "IsVector" << YAML::Value << field.Field.IsVector;

        if (field.Field.IsVector) {
          out << YAML::Key << "ElementSize" << YAML::Value
              << field.Field.ElementSize;
          std::string fieldTypeStr = DataTypeToString(field.Field.ElementType);
          if (fieldTypeStr.empty()) {
            fieldTypeStr = field.Field.ElementTypeName;
          }
          out << YAML::Key << "ElementType" << YAML::Value << fieldTypeStr;
          out << YAML::Key << "Value" << YAML::Value;
          out << YAML::BeginSeq; // Field Vector Start
          for (size_t i = 0; i < field.VectorData.GetCount(); i++) {
            const void *elementPtr =
                field.VectorData.GetElementPointer(i, field.Field.ElementSize);
            SerializeEntityScriptVector(out, field.Field.ElementType,
                                        elementPtr);
          }
          out << YAML::EndSeq; // Field Vector End
        } else {
          out << YAML::Key << "Value";
          switch (field.Field.Type) {
            case DataType::Short:
              out << YAML::Value << field.GetValue<int16_t>();
              break;
            case DataType::UShort:
              out << YAML::Value << field.GetValue<uint16_t>();
              break;
            case DataType::Int:
              out << YAML::Value << field.GetValue<int32_t>();
              break;
            case DataType::UInt:
              out << YAML::Value << field.GetValue<uint32_t>();
              break;
            case DataType::Long:
              out << YAML::Value << field.GetValue<int64_t>();
              break;
            case DataType::ULong:
              out << YAML::Value << field.GetValue<uint64_t>();
              break;
            case DataType::Float:
              out << YAML::Value << field.GetValue<float>();
              break;
            case DataType::Double:
              out << YAML::Value << field.GetValue<double>();
              break;
            case DataType::Bool:
              out << YAML::Value << field.GetValue<bool>();
              break;
            case DataType::Vector2:
              out << YAML::Value << field.GetValue<glm::vec2>();
              break;
            case DataType::Vector3:
              out << YAML::Value << field.GetValue<glm::vec3>();
              break;
            case DataType::Vector4:
              out << YAML::Value << field.GetValue<glm::vec4>();
              break;
            case DataType::Entity:
            case DataType::Texture2D:
            case DataType::Prefab:
            case DataType::ScriptComponent:
              out << YAML::Value << field.GetValue<uint64_t>();
              break;
            default:
              break;
          }
          sc.EditDirty = false;
        }
        out << YAML::EndMap; // Field Map End
      }
      out << YAML::EndSeq; // Fields End
      out << YAML::EndMap; // ScriptComponent End
      out << YAML::EndMap; // Script Component End
    }
    container.EditDirty = false;
    out << YAML::EndSeq; // ScriptComponentContainer end
  }

  if (entity.HasComponent<RelationshipComponent>()) {
    auto &rc = entity.GetComponent<RelationshipComponent>();
    out << YAML::Key << "RelationshipComponent" << YAML::Value;
    out << YAML::BeginMap;
    out << YAML::Key << "ParentID" << YAML::Value << rc.Parent.uuid();
    out << YAML::Key << "Children" << YAML::Value;
    out << YAML::BeginSeq;
    for (auto &child : rc.Children) {
      Entity e;
      auto *scene = entity.GetScene();
      e = scene->GetEntityByUUID(child);
      SerializeEntity(out, e);
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;
  }

  out << YAML::EndMap; // Entity
}

void PrefabSerializer::Serialize(const std::filesystem::path &filepath, Entity rootEntity) {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Prefab" << YAML::Value << "CatEngine Prefab";
  out << YAML::Key << "Entities" << YAML::BeginSeq;

  if (!rootEntity) {
    auto root = m_Prefab->m_PrefabScene->GetRootEntities()[0];
    rootEntity = m_Prefab->m_PrefabScene->GetEntityByUUID(root);
  }

  SerializeEntity(out, rootEntity);

  out << YAML::EndSeq;
  out << YAML::EndMap;

  std::ofstream fout(filepath);
  fout << out.c_str();
}
void PrefabSerializer::DeserializeEntityScriptVector(YAML::Node &vectorNode,
                                               const DataType elementType,
                                               size_t elementSize,
                                               VectorBuffer &outVectorData) {
  if (!vectorNode.IsSequence())
    return;

  size_t count = vectorNode.size();
  // Note: Ensure your VectorData class has a resize/allocation method if count
  // differs, or that outVectorData is pre-allocated to match the YAML sequence
  // size. e.g., outVectorData.Resize(count, elementSize);

  for (const auto &element : vectorNode) {
    switch (elementType) {
    case DataType::Short: {
      int16_t value = element.as<int16_t>();
      outVectorData.PushBack(sizeof(int16_t), &value);
      break;
    }
    case DataType::UShort: {
      uint16_t value = element.as<uint16_t>();
      outVectorData.PushBack(sizeof(uint16_t), &value);
      break;
    }
    case DataType::Int: {
      int32_t value = element.as<int32_t>();
      outVectorData.PushBack(sizeof(int32_t), &value);
      break;
    }
    case DataType::UInt: {
      uint32_t value = element.as<uint32_t>();
      outVectorData.PushBack(sizeof(uint32_t), &value);
      break;
    }
    case DataType::Long: {
      int64_t value = element.as<int64_t>();
      outVectorData.PushBack(sizeof(int64_t), &value);
      break;
    }
    case DataType::ULong: {
      uint64_t value = element.as<uint64_t>();
      outVectorData.PushBack(sizeof(uint64_t), &value);
      break;
    }
    case DataType::Float: {
      float value = element.as<float>();
      outVectorData.PushBack(sizeof(float), &value);
      break;
    }
    case DataType::Double: {
      double value = element.as<double>();
      outVectorData.PushBack(sizeof(double), &value);
      break;
    }
    case DataType::Bool: {
      bool value = element.as<bool>();
      outVectorData.PushBack(sizeof(bool), &value);
      break;
    }
    case DataType::Vector2: {
      glm::vec2 value = element.as<glm::vec2>();
      outVectorData.PushBack(sizeof(glm::vec2), &value);
      break;
    }
    case DataType::Vector3: {
      glm::vec3 value = element.as<glm::vec3>();
      outVectorData.PushBack(sizeof(glm::vec3), &value);
      break;
    }
    case DataType::Vector4: {
      glm::vec4 value = element.as<glm::vec4>();
      outVectorData.PushBack(sizeof(glm::vec4), &value);
      break;
    }
    case DataType::Entity:
    case DataType::Texture2D:
    case DataType::Prefab:
    case DataType::ScriptComponent: {
      uint64_t value = element.as<uint64_t>();
      outVectorData.PushBack(sizeof(uint64_t), &value);
      break;
    }
    default:
      break;
    }
  }
}

void PrefabSerializer::DeserializeEntity(YAML::Node &entity, UUID parent) {
  uint64_t uuid = entity["Entity"].as<uint64_t>();

  std::string name;
  auto nameComponent = entity["NameComponent"];
  if (nameComponent)
    name = nameComponent["Name"].as<std::string>();

  Entity deserializedEntity = m_Prefab->m_PrefabScene->CreateEntityWithUUID(uuid, name, parent);

  if (nameComponent && deserializedEntity.HasComponent<NameComponent>()) {
    deserializedEntity.GetComponent<NameComponent>().PrefabDirty =
        nameComponent["PrefabDirty"].as<bool>(false);
  }

  std::string tag;
  auto tagComponent = entity["TagComponent"];
  if (tagComponent) {
    tag = tagComponent["Tag"].as<std::string>();
  }
  auto &tc = deserializedEntity.GetComponent<TagComponent>();
  tc.Tag = tag;
  tc.PrefabDirty = tagComponent ? tagComponent["PrefabDirty"].as<bool>(false) : false;

  std::string layer;
  auto layerComponent = entity["LayerComponent"];
  if (layerComponent) {
    layer = layerComponent["Layer"].as<std::string>();
  }
  auto &lc = deserializedEntity.GetComponent<LayerComponent>();
  lc.Layer = layer;
  lc.PrefabDirty = layerComponent ? layerComponent["PrefabDirty"].as<bool>(false) : false;

  auto objectStateComponent = entity["ObjectStateComponent"];
  if (objectStateComponent) {
    auto &osc = deserializedEntity.GetOrAddComponent<ObjectStateComponent>();
    osc.Locked = objectStateComponent["Locked"].as<bool>(false);
    osc.SelfLocked = objectStateComponent["SelfLocked"].as<bool>(false);
    osc.HierarchyLocked = objectStateComponent["HierarchyLocked"].as<bool>(false);

    osc.Active = objectStateComponent["Active"].as<bool>(true);
    osc.SelfActive = objectStateComponent["SelfActive"].as<bool>(true);
    osc.HierarchyActive = objectStateComponent["HierarchyActive"].as<bool>(true);

    osc.PrefabDirty = objectStateComponent["PrefabDirty"].as<bool>(false);
    osc.PrefabAdded = objectStateComponent["PrefabAdded"].as<bool>(false);
  }

  auto prefabLinkComponent = entity["PrefabLinkComponent"];
  if (prefabLinkComponent) {
    auto &plc = deserializedEntity.GetOrAddComponent<PrefabLinkComponent>();
    plc.PrefabID = prefabLinkComponent["PrefabID"].as<UUID>();
    plc.LastKnownUpdate = prefabLinkComponent["LastKnownUpdate"].as<uint64_t>(0);
    plc.PrefabDirty = prefabLinkComponent["PrefabDirty"].as<bool>(false);
  }

  auto transformComponent = entity["TransformComponent"];
  if (transformComponent) {
    auto &trc = deserializedEntity.GetOrAddComponent<TransformComponent>();
    trc.LocalPosition =
        transformComponent["LocalPosition"].as<glm::vec3>(glm::vec3{0, 0, 0});
    trc.LocalRotation =
        transformComponent["LocalRotation"].as<glm::vec3>(glm::vec3{0, 0, 0});
    trc.LocalScale =
        transformComponent["LocalScale"].as<glm::vec3>(glm::vec3{1, 1, 1});
    trc.PrefabDirty = transformComponent["PrefabDirty"].as<bool>(false);
  }

  auto spriteRendererComponent = entity["SpriteRendererComponent"];
  if (spriteRendererComponent) {
    auto &src = deserializedEntity.GetOrAddComponent<SpriteRendererComponent>();
    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
    src.Texture = spriteRendererComponent["Texture"].as<AssetHandle>();
    src.TilingFactor = spriteRendererComponent["Tiling"].as<float>();
    src.PrefabDirty = spriteRendererComponent["PrefabDirty"].as<bool>(false);
  }

  auto circleRendererComponent = entity["CircleRendererComponent"];
  if (circleRendererComponent) {
    auto &crc = deserializedEntity.GetOrAddComponent<CircleRendererComponent>();
    crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
    crc.Thickness = circleRendererComponent["Thickness"].as<float>();
    crc.Fade = circleRendererComponent["Fade"].as<float>();
    crc.PrefabDirty = circleRendererComponent["PrefabDirty"].as<bool>(false);
  }

  auto cameraComponent = entity["CameraComponent"];
  if (cameraComponent) {
    auto &cc = deserializedEntity.AddComponent<CameraComponent>();

    const auto &cameraProps = cameraComponent["Camera"];
    cc.Camera.SetProjectionType(
        (SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

    cc.Camera.SetPerspectiveVerticalFov(
        cameraProps["PerspectiveFOV"].as<float>());
    cc.Camera.SetPerspectiveNearClip(
        cameraProps["PerspectiveNear"].as<float>());
    cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
    cc.Camera.SetOrthographicNearClip(
        cameraProps["OrthographicNear"].as<float>());
    cc.Camera.SetOrthographicFarClip(
        cameraProps["OrthographicFar"].as<float>());

    cc.Primary = cameraComponent["Primary"].as<bool>();
    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
    cc.PrefabDirty = cameraComponent["PrefabDirty"].as<bool>(false);
  }

  auto rigidBody2DComponent = entity["Rigidbody2DComponent"];
  if (rigidBody2DComponent) {
    auto &rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
    rb2d.Type = (Rigidbody2DComponent::BodyType)rigidBody2DComponent["BodyType"]
                    .as<int>();
    rb2d.FixedRotation = rigidBody2DComponent["FixedRotation"].as<bool>();
    rb2d.PrefabDirty = rigidBody2DComponent["PrefabDirty"].as<bool>(false);
  }

  auto boxCollider2D = entity["BoxCollider2DComponent"];
  if (boxCollider2D) {
    auto &bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();

    bc2d.Offset = boxCollider2D["Offset"].as<glm::vec2>();
    bc2d.Size = boxCollider2D["Size"].as<glm::vec2>();
    bc2d.Rotation = boxCollider2D["Rotation"].as<float>();
    bc2d.ShowColliderBounds = boxCollider2D["ShowColliderBounds"].as<bool>();
    bc2d.IsTrigger = boxCollider2D["IsTrigger"].as<bool>(false);
    bc2d.Density = boxCollider2D["Density"].as<float>();
    bc2d.Friction = boxCollider2D["Friction"].as<float>();
    bc2d.Restitution = boxCollider2D["Restitution"].as<float>();
    bc2d.RestitutionThreshold =
        boxCollider2D["RestitutionThreshold"].as<float>();
    bc2d.PrefabDirty = boxCollider2D["PrefabDirty"].as<bool>(false);
  }

  auto circleCollider2D = entity["CircleCollider2DComponent"];
  if (circleCollider2D) {
    auto &cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();

    cc2d.Offset = circleCollider2D["Offset"].as<glm::vec2>();
    cc2d.Radius = circleCollider2D["Radius"].as<float>();
    cc2d.ShowColliderBounds = circleCollider2D["ShowColliderBounds"].as<bool>();
    cc2d.IsTrigger = circleCollider2D["IsTrigger"].as<bool>(false);
    cc2d.Density = circleCollider2D["Density"].as<float>();
    cc2d.Friction = circleCollider2D["Friction"].as<float>();
    cc2d.Restitution = circleCollider2D["Restitution"].as<float>();
    cc2d.RestitutionThreshold =
        circleCollider2D["RestitutionThreshold"].as<float>();
    cc2d.PrefabDirty = circleCollider2D["PrefabDirty"].as<bool>(false);
  }

  auto scriptComponentContainer = entity["ScriptComponentContainer"];
  if (scriptComponentContainer) {
    auto &scriptEngine = ScriptEngine::GetMutable();
    auto &scc = deserializedEntity.AddComponent<ScriptComponentContainer>();
    for (auto scriptEntry : scriptComponentContainer) {
      auto scriptComponent = scriptEntry["ScriptComponent"];
      if (scriptComponent) {
        uint64_t scriptID = scriptComponent["ScriptID"].as<uint64_t>();
        std::string className = scriptComponent["ClassName"].as<std::string>();
        ScriptComponent &sc = deserializedEntity.AddScriptComponent();
        sc.ScriptID = scriptID;
        sc.ClassName = className;
        sc.PrefabDirty = scriptComponent["PrefabDirty"].as<bool>(false);

        scc.LoadedScriptNames.push_back(className);

        auto &fieldMap = scriptEngine.GetScriptFieldMap(uuid)[scriptID];

        auto fieldsSeq = scriptComponent["Fields"];
        if (fieldsSeq) {
          for (auto field : fieldsSeq) {
            uint64_t fieldID = field["FieldID"].as<uint64_t>();
            ScriptFieldInstance &sfi = fieldMap[fieldID];

            sfi.Field.Name = field["FieldName"].as<std::string>();
            sfi.Field.Type =
                StringToDataType(field["FieldType"].as<std::string>());
            sfi.Field.TypeName = field["FieldType"].as<std::string>();
            sfi.Field.Size = field["FieldSize"].as<uint64_t>();

            sfi.Field.IsVector = field["IsVector"].as<bool>();

            if (sfi.Field.IsVector) {
              sfi.Field.ElementSize = field["ElementSize"].as<uint64_t>();
              sfi.Field.ElementType =
                  StringToDataType(field["ElementType"].as<std::string>());
              sfi.Field.ElementTypeName =
                  field["ElementType"].as<std::string>();
              auto valueSeq = field["Value"];
              if (valueSeq && valueSeq.IsSequence()) {
                DeserializeEntityScriptVector(valueSeq, sfi.Field.ElementType,
                                              sfi.Field.ElementSize,
                                              sfi.VectorData);
              }
            } else {
              sfi.TrivialData.Allocate(sfi.Field.Size);

              switch (sfi.Field.Type) {
              case DataType::Float:
                sfi.SetValue(field["Value"].as<float>());
                break;
              case DataType::Double:
                sfi.SetValue(field["Value"].as<double>());
                break;
              case DataType::Bool:
                sfi.SetValue(field["Value"].as<bool>());
                break;
              case DataType::Short:
                sfi.SetValue(field["Value"].as<int16_t>());
                break;
              case DataType::UShort:
                sfi.SetValue(field["Value"].as<uint16_t>());
                break;
              case DataType::Int:
                sfi.SetValue(field["Value"].as<int32_t>());
                break;
              case DataType::UInt:
                sfi.SetValue(field["Value"].as<uint32_t>());
                break;
              case DataType::Long:
                sfi.SetValue(field["Value"].as<int64_t>());
                break;
              case DataType::ULong:
                sfi.SetValue(field["Value"].as<uint64_t>());
                break;
              case DataType::Vector2:
                sfi.SetValue(field["Value"].as<glm::vec2>());
                break;
              case DataType::Vector3:
                sfi.SetValue(field["Value"].as<glm::vec3>());
                break;
              case DataType::Vector4:
                sfi.SetValue(field["Value"].as<glm::vec4>());
                break;
              case DataType::Entity:
              case DataType::Texture2D:
              case DataType::Prefab:
              case DataType::ScriptComponent:
                sfi.SetValue(field["Value"].as<uint64_t>());
                break;
              default:
                break;
              }
            }
          }
        }
      }
    }
  }

  auto relationshipComponent = entity["RelationshipComponent"];
  if (relationshipComponent) {
    auto &rc = deserializedEntity.GetComponent<RelationshipComponent>();
    rc.Parent = parent;
    for (auto child : relationshipComponent["Children"]) {
      DeserializeEntity(child, uuid);
    }
  }
}


bool PrefabSerializer::Deserialize(const std::filesystem::path &filepath) {
  CE_PROFILE_FUNCTION();

  YAML::Node data;
  try {
    data = YAML::LoadFile(filepath);
  } catch (YAML::ParserException ex) {
    CE_API_ERROR("Failed to load .catengine file '{0}'\n       {1}", filepath.string(),
                 ex.what());
    return false;
  }

  auto entities = data["Entities"];
  if (entities) {
    for (auto entity : entities) {
      DeserializeEntity(entity, UUID(0));
    }
  }
  m_Prefab->m_PrefabScene->RegisterParents();
  m_Prefab->m_PrefabScene->m_Handle = m_Prefab->m_Handle;
  return true;
}

} // namespace CatEngine
