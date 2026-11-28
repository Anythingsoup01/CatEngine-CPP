#pragma once

#include "UITypes.h"

namespace CatEngine {

struct EventListenerComponent {
  uint32_t Mask = UIMask::All;
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
