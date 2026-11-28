#pragma once

#include "UITypes.h"

namespace CatEngine {

struct SliderComponent {
  float MinValue = 0.0f, MaxValue = 1.0f, Value = 0.5f;
  bool WholeNumbers = false;
  UISliderDirection Direction = UISliderDirection::LeftToRight;
  bool EditDirty = false;
  bool PrefabDirty = false;

  float Normalized() const {
    return MaxValue > MinValue ? (Value - MinValue) / (MaxValue - MinValue) : 0.0f;
  }
};

}
