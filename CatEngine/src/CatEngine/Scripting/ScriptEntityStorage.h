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

  DynamicVector,

  Entity,

  Texture2D,

  ScriptComponent,
};


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
