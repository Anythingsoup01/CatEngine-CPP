#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/UUID.h"

namespace CatEngine {

struct PrefabLinkComponent {
  AssetHandle PrefabID = 0;
  bool PrefabDirty = false;

  uint64_t LastKnownUpdate = 0;

  std::vector<std::string> DeletedEntities;

  PrefabLinkComponent() = default;
  PrefabLinkComponent(AssetHandle prefabID, UUID sourceEntityID, uint64_t lastKnownUpdate)
    : PrefabID(prefabID), LastKnownUpdate(lastKnownUpdate) {}
};

}
