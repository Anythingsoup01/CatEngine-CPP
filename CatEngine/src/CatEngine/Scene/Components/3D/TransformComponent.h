#pragma once

#include "CatEngine/Math/Math.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CatEngine {
struct TransformComponent {
  glm::vec3 LocalPosition = {0.0f, 0.0f, 0.0f};
  glm::vec3 LocalRotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 LocalScale = {1.0f, 1.0f, 1.0f};

  glm::vec3 WorldPosition = {0.0f, 0.0f, 0.0f};
  glm::vec3 WorldRotation = {0.0f, 0.0f, 0.0f};
  glm::vec3 WorldScale = {1.0f, 1.0f, 1.0f};


  TransformComponent() = default;
  TransformComponent(const TransformComponent &) = default;
  TransformComponent(const glm::vec3 &position) : WorldPosition(position) {}

  ~TransformComponent() = default;

  // Computes the local transformation matrix
  glm::mat4 GetLocalTransform() const {
    glm::mat4 rotation = glm::toMat4(glm::quat(LocalRotation));
    return glm::translate(glm::mat4(1.f), LocalPosition) * rotation *
    glm::scale(glm::mat4(1.f), LocalScale);
  }

  // Computes the world transform given a parent's world matrix
  glm::mat4 GetWorldTransform(const glm::mat4 &parentWorldMatrix = glm::mat4(1.0f)) const {
    return parentWorldMatrix * GetLocalTransform();
  }

  TransformComponent RelativeTo(const TransformComponent &other) {
    TransformComponent result = *this;

    // 1. Get the parent's world transform matrix
    glm::mat4 parentMatrix = other.GetWorldTransform();

    // 2. Get this component's current world transform matrix
    glm::mat4 worldMatrix = GetWorldTransform();

    // 3. Compute the local transform matrix: Local = Inverse(Parent) * World
    glm::mat4 localMatrix = glm::inverse(parentMatrix) * worldMatrix;

    // 4. Decompose the resulting local matrix into LocalPosition, LocalRotation, and LocalScale
    Math::DecomposeTransform(localMatrix, result.LocalPosition, result.LocalRotation, result.LocalScale);

    return result;
  }
};
} // namespace CatEngine
