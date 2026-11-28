#pragma once

namespace CatEngine {

struct DropdownComponent {
  std::vector<std::string> Options{"Option A", "Option B", "Option C"};
  int32_t Value = 0;
  float ItemHeight = 32.0f; // canvas units
  bool EditDirty = false;
  bool PrefabDirty = false;
};

}
