#pragma once

#include "CatEngine/AssetManager/Asset.h"
#include "CatEngine/Core/UUID.h"

namespace CatEngine {

struct PrefabLinkComponent {
  AssetHandle PrefabID = 0;
  UUID SourceEntityID = 0;

  PrefabLinkComponent() = default;
  PrefabLinkComponent(AssetHandle prefabID, UUID sourceEntityID)
    : PrefabID(prefabID), SourceEntityID(sourceEntityID) {}
};

}
