#pragma once

#include "CatEngine/Core/UUID.h"
namespace CatEngine {

struct RelationshipComponent {
  UUID Parent = 0;
  std::vector<UUID> Children;
};

}
