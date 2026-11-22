#pragma once

#include <Capybara/Core/ReflectionTypes.h>

#include <glm/glm.hpp>

namespace CatEngine {

class ScriptClass;
class ScriptInstance;

enum class DataType {
  Short,
  UShort,
  Int,
  UInt,
  Long,
  ULong,

  Float,
  Double,

  Bool,

  Vector2,
  Vector3,
  Vector4,

  DynamicVector,

  Entity,

  Texture2D,
  Prefab,

  ScriptComponent,
};

static std::string DataTypeToString(const DataType &type) {
  switch (type) {
  case DataType::Short:
    return "Short";
  case DataType::UShort:
    return "UShort";
  case DataType::Int:
    return "Int";
  case DataType::UInt:
    return "UInt";
  case DataType::Long:
    return "Long";
  case DataType::ULong:
    return "ULong";
  case DataType::Float:
    return "Float";
  case DataType::Double:
    return "Double";
  case DataType::Bool:
    return "Bool";
  case DataType::Vector2:
    return "Vector2";
  case DataType::Vector3:
    return "Vector3";
  case DataType::Vector4:
    return "Vector4";
  case DataType::DynamicVector:
    return "DynamicVector";
  case DataType::Entity:
    return "Entity";
  case DataType::Texture2D:
    return "Texture";
  case DataType::Prefab:
    return "Prefab";

  default:
    return "";
  }
}

static DataType StringToDataType(const std::string &typeStr) {
  if (typeStr == "Short")
    return DataType::Short;
  if (typeStr == "UShort")
    return DataType::UShort;
  if (typeStr == "Int")
    return DataType::Int;
  if (typeStr == "UInt")
    return DataType::UInt;
  if (typeStr == "Long")
    return DataType::Long;
  if (typeStr == "ULong")
    return DataType::ULong;
  if (typeStr == "Float")
    return DataType::Float;
  if (typeStr == "Double")
    return DataType::Double;
  if (typeStr == "Bool")
    return DataType::Bool;
  if (typeStr == "Vector2")
    return DataType::Vector2;
  if (typeStr == "Vector3")
    return DataType::Vector3;
  if (typeStr == "Vector4")
    return DataType::Vector4;
  if (typeStr == "DynamicVector")
    return DataType::DynamicVector;
  if (typeStr == "Entity")
    return DataType::Entity;
  if (typeStr == "Texture")
    return DataType::Texture2D;
  if (typeStr == "Prefab")
    return DataType::Prefab;

  return DataType::ScriptComponent;
}


struct ScriptField {
  std::string Name;
  DataType Type;
  std::string TypeName;
  uint64_t Size;

  bool IsVector = false;
  DataType ElementType;
  std::string ElementTypeName;
  uint64_t ElementSize;
  uint64_t ElementCount;

  std::vector<uint8_t> DefaultValue;
};

struct ScriptFieldInstance {
  ScriptField Field;
  Buffer TrivialData;
  VectorBuffer VectorData;

  ScriptFieldInstance() = default;

  template <typename T> T GetValue() { return TrivialData.Read<T>(); }

  template <typename T> T GetValue() const { return TrivialData.Read<T>(); }

  template <typename T> void SetValue(T value) {
    TrivialData.Write(&value, Field.Size);
  }

  void SetVectorValue(const std::vector<uint8_t> &value) {
    if (!Field.IsVector)
      return;

    VectorData.Data = value;
  }

  void *GetVectorValue(uint64_t index) {
    if (!Field.IsVector || VectorData.GetCount() == 0)
      return nullptr;

    if (index >= VectorData.GetCount())
      return nullptr;

    return static_cast<void *>(VectorData.GetElementPointer(index, Field.ElementSize));
  }
};

// Key by script name hash, field instances keyed by field hash
using FieldMap = std::unordered_map<uint64_t, ScriptFieldInstance>;
using ScriptFieldMap = std::unordered_map<uint64_t, FieldMap>;
using EntityClassMap = std::unordered_map<uint64_t, Ref<ScriptClass>>;
using EntityScriptInstanceMap = std::unordered_map<uint64_t, Ref<ScriptInstance>>;

} // namespace CatEngine
