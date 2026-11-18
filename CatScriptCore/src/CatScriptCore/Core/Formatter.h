#pragma once

#include <format>
#include <string_view>
#include "CatScriptCore/Scene/Component/AllComponents.h"

// TODO: Implement all vector proxies from all components

template<>
struct std::formatter<CatRuntime::Layer> : std::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const CatRuntime::Layer& layer, FormatContext& ctx) const {
    // Automatically invoke your custom conversion operator
    const char* layerName = static_cast<const char*>(layer);
    return std::formatter<std::string_view>::format(layerName ? layerName : "(null)", ctx);
  }
};

template<>
struct std::formatter<CatRuntime::Tag> : std::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const CatRuntime::Tag& tag, FormatContext& ctx) const {
    // Automatically invoke your custom conversion operator
    const char* tagName = static_cast<const char*>(tag);
    return std::formatter<std::string_view>::format(tagName ? tagName : "(null)", ctx);
  }
};
