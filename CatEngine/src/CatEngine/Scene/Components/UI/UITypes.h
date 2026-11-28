#pragma once

#include "CatEngine/Core/UUID.h"

#include <glm/glm.hpp>

namespace CatEngine {

enum class UIAnchorPreset : uint8_t {
  TopLeft = 0,
  TopCenter,
  TopRight,
  MiddleLeft,
  Center,
  MiddleRight,
  BottomLeft,
  BottomCenter,
  BottomRight,
  StretchAll,
  Custom
};

struct UIScreenRect {
  glm::vec2 Min{0.0f, 0.0f};
  glm::vec2 Max{0.0f, 0.0f};

  glm::vec2 GetSize() const { return Max - Min; }
  glm::vec2 GetCenter() const { return (Min + Max) * 0.5f; }
  bool IsValid() const { return Max.x > Min.x && Max.y > Min.y; }

  bool Contains(const glm::vec2 &p) const {
    return p.x >= Min.x && p.x <= Max.x && p.y >= Min.y && p.y <= Max.y;
  }
};

enum class UIEventType : uint8_t {
  PointerEnter, PointerExit,
  PointerDown, PointerUp, PointerClick,
  BeginDrag, Drag, EndDrag,
  Scroll,
  Select, Deselect, Submit, Cancel, Move
};

enum class UINavDirection : uint8_t { None, Left, Right, Up, Down };
enum class UITransition   : uint8_t { None, ColorTint };

// Which events an entity wants. Selectables imply Hover | Press | Click.
namespace UIMask {
constexpr uint32_t Hover  = 1u << 0; // PointerEnter / PointerExit
constexpr uint32_t Press  = 1u << 1; // PointerDown / PointerUp
constexpr uint32_t Click  = 1u << 2; // PointerClick
constexpr uint32_t Drag   = 1u << 3; // BeginDrag / Drag / EndDrag
constexpr uint32_t Scroll = 1u << 4;
constexpr uint32_t All    = 0x1F;
}

struct UIEvent {
  UIEventType Type{};
  UUID Target = 0;
  glm::vec2 Position{0.0f}; // viewport pixels, Y-up
  glm::vec2 Delta{0.0f};    // Drag: movement this frame. Scroll: wheel delta.
  uint8_t Button = 0;       // 0 left, 1 right, 2 middle
  int ClickCount = 0;       // PointerClick only (2 = double click)
  UINavDirection Direction = UINavDirection::None; // Move only
};

// Fed by your viewport panel once per frame. One-shot fields are consumed on use.
struct UIInputFrame {
  bool PointerValid = false;       // pointer is over the game view
  glm::vec2 PointerPos{0.0f};      // viewport pixels, Y-up
  std::array<bool, 3> Buttons{};   // held state: left, right, middle
  glm::vec2 Scroll{0.0f};          // one-shot
  UINavDirection Move = UINavDirection::None; // one-shot (use key-repeat)
  bool Submit = false, Cancel = false;        // one-shot
  bool TabNext = false, TabPrev = false;      // one-shot
};

// Inherited down the hierarchy during layout.
struct UIGroupState {
  float Alpha = 1.0f;
  bool Interactable = true;
  bool BlocksRaycasts = true;
};

struct UIDrawItem {
  UUID Entity = 0;
  float Scale = 1.0f;          // canvas scale (will matter for text later)
  float Alpha = 1.0f;          // accumulated CanvasGroup alpha
  bool Interactable = true;    // accumulated CanvasGroup state
  bool BlocksRaycasts = true;
  UUID TintSource = 0;         // selectable whose tint applies to this node's graphics
};

struct UIHandlerDesc { UIEventType Type; const char *Method; uint32_t Mask; };

inline constexpr UIHandlerDesc kUIHandlers[] = {
  {UIEventType::PointerEnter, "OnPointerEnter", UIMask::Hover},
  {UIEventType::PointerExit,  "OnPointerExit",  UIMask::Hover},
  {UIEventType::PointerDown,  "OnPointerDown",  UIMask::Press},
  {UIEventType::PointerUp,    "OnPointerUp",    UIMask::Press},
  {UIEventType::PointerClick, "OnPointerClick", UIMask::Click},
  {UIEventType::BeginDrag,    "OnBeginDrag",    UIMask::Drag},
  {UIEventType::Drag,         "OnDrag",         UIMask::Drag},
  {UIEventType::EndDrag,      "OnEndDrag",      UIMask::Drag},
  {UIEventType::Scroll,       "OnScroll",       UIMask::Scroll},
  {UIEventType::Select,       "OnSelect",       0},
  {UIEventType::Deselect,     "OnDeselect",     0},
  {UIEventType::Submit,       "OnSubmit",       0},
  {UIEventType::Cancel,       "OnCancel",       0},
  {UIEventType::Move,         "OnMove",         0},
};

}
