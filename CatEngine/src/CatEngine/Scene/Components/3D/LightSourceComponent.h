#pragma once

#include <glm/glm.hpp>

namespace CatEngine {
enum class LightType { None = 0, Directional = 1, Point = 2, Spot = 3 };

struct DirectionalLightComponent {
  glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
  float Intensity = 1.0f;
  bool CastShadows = true;
  bool SoftShadows = true;
  float LightSize = 0.5f; // For PCSS
  float ShadowAmount = 1.0f;
};

struct PointLightComponent {
  glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
  float Intensity = 1.0f;
  float LightSize = 0.5f; // For PCSS
  float MinRadius = 1.f;
  float Radius = 10.f;
  bool CastsShadows = true;
  bool SoftShadows = true;
  float Falloff = 1.f;
};

struct SpotLightComponent {
  glm::vec3 Radiance{1.0f};
  float Intensity = 1.0f;
  float Range = 10.0f;
  float Angle = 60.0f;
  float AngleAttenuation = 5.0f;
  bool CastsShadows = false;
  bool SoftShadows = false;
  float Falloff = 1.0f;
};
} // namespace CatEngine
