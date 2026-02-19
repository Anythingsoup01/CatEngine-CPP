#pragma once

#include <glm/glm.hpp>

struct QuadVertex {
  glm::vec3 Position;
  glm::vec4 Color;
  glm::vec2 TexCoord;
  float TexIndex;
  float TilingFactor;
  int EntityID;
};

struct CircleVertex {
  glm::vec3 WorldPosition;
  glm::vec3 LocalPosition;
  glm::vec4 Color;
  float Thickness;
  float Fade;
  int EntityID;
};

struct LineVertex {
  glm::vec3 Position;
  glm::vec4 Color;
  int EntityID;
};
