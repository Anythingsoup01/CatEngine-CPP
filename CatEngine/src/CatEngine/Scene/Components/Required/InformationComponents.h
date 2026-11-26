#pragma once

#include <string>

#include "CatEngine/Core/UUID.h"

namespace CatEngine {

struct IDComponent {
  UUID ID;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;


  IDComponent() = default;
  IDComponent(const IDComponent &) = default;
  IDComponent(const UUID &id) : ID(id) {}

  ~IDComponent() = default;
};

struct TagComponent {
  std::string Tag;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  TagComponent() = default;
  TagComponent(const TagComponent &) = default;
  TagComponent(const std::string &tag) : Tag(tag) {}

  ~TagComponent() = default;
};

struct LayerComponent {
  std::string Layer;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  LayerComponent() = default;
  LayerComponent(const LayerComponent &) = default;
  LayerComponent(const std::string &layer) : Layer(layer) {}

  ~LayerComponent() = default;
};

struct NameComponent {
  std::string Name;

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;

  NameComponent() = default;
  NameComponent(const NameComponent &) = default;
  NameComponent(const std::string &name) : Name(name) {}

  ~NameComponent() = default;
};

struct ObjectStateComponent {
  bool Locked = false;
  bool SelfLocked = false;
  bool HierarchyLocked = false;

  bool Active = true;       // Local toggle
  bool SelfActive = true;   // True if explicitly enabled by user
  bool HierarchyActive = true; // Cached flag: true if parent chain is active

  bool RuntimeDirty = false;
  bool EditDirty = false;
  bool PrefabDirty = false;
  bool PrefabAdded = false;

  std::vector<std::string> DeletedComponents;

  ObjectStateComponent() = default;
  ObjectStateComponent(const ObjectStateComponent &) = default;
};

} // namespace CatEngine
