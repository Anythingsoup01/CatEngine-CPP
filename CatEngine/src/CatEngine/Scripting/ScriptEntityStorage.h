#pragma once

#include <Capybara/Core/ReflectionTypes.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <unordered_map>

#include "CatEngine/Core/UUID.h"

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

  Entity,

  Texture2D,

  ScriptComponent,
};

inline uint64_t TypeToSize(const DataType &type) {
  switch (type) {
    case DataType::Short:
      return sizeof(int16_t);
    case DataType::UShort:
      return sizeof(uint16_t);
    case DataType::Int:
      return sizeof(int32_t);
    case DataType::UInt:
      return sizeof(uint32_t);
    case DataType::Long:
      return sizeof(int64_t);
    case DataType::ULong:
      return sizeof(uint64_t);
    case DataType::Float:
      return sizeof(float);
    case DataType::Double:
      return sizeof(bool);
    case DataType::Bool:
      return sizeof(bool);
    case DataType::Vector2:
      return sizeof(float) * 2;
    case DataType::Vector3:
      return sizeof(float) * 3;
    case DataType::Vector4:
      return sizeof(float) * 4;
    case DataType::Entity:
      return sizeof(UUID);
    case DataType::Texture2D:
      return sizeof(UUID);
    case DataType::ScriptComponent:
      return sizeof(UUID);

  }
}

struct ScriptField {
  std::string Name;
  DataType Type;
  std::string TypeName;
  uint64_t Size;
  std::vector<uint8_t> DefaultValue;
};

struct ScriptFieldInstance {
  ScriptField Field;
  Buffer Data;

  ScriptFieldInstance() { Data.Allocate(128); }

  template <typename T> T GetValue() {
    static_assert(sizeof(T) <= 128, "Type to large!");
    return Data.Read<T>();
  }

  template <typename T> T GetValue() const {
    static_assert(sizeof(T) <= 128, "Type to large!");
    return Data.Read<T>();
  }

  template <typename T> void SetValue(T value) {
    static_assert(sizeof(T) <= 128, "Type to large!");
    Data.Write(&value, TypeToSize(Field.Type));
  }

};

// Key by script name hash, field instances keyed by field hash
using FieldMap = std::unordered_map<uint64_t, ScriptFieldInstance>;
using ScriptFieldMap = std::unordered_map<uint64_t, FieldMap>;
using EntityClassMap = std::unordered_map<uint64_t, Ref<ScriptClass>>;
using EntityScriptInstanceMap = std::unordered_map<uint64_t, Ref<ScriptInstance>>;

} // namespace CatEngine
