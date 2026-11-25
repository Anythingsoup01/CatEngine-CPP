#pragma once

#include "ContactListener.h"

#include "CatEngine/Core/TimeStep.h"

#include <glm/glm.hpp>
#include <box2d/b2_body.h>

namespace CatEngine {

class Scene;

class Entity;

class Physics2D {

public:
  void Start(Scene *context);

  void Update(Time ts);

  void Stop();

  void ApplyForce(const Entity &entity, const glm::vec2 &impulse, const glm::vec2 &point, bool wake);
  void ApplyForceToCenter(const Entity &entity, const glm::vec2 &impulse, bool wake);
  void ApplyLinearImpulse(const Entity &entity, const glm::vec2 &impulse, const glm::vec2 &point, bool wake);
  void ApplyLinearImpulseToCenter(const Entity &entity, const glm::vec2 &impulse, bool wake);

  void EnableCollider(const Entity &entity, bool enable);

  void EnableRigidbody(const Entity &entity, bool enable);

  static Physics2D &Get();
private:

  void SyncColliders();

  void ExecutePhysicsCommandQueue();

private:
  Scene *m_Context;
  b2World *m_PhysicsWorld = nullptr;
  ContactListener m_ContactListenter;

  std::vector<std::function<void()>> m_PhysicsCommandQueue;
  std::mutex m_PhysicsCommandQueueMutex;
};

} // namespace CatEngine
