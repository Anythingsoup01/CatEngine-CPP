#include "Physics2D.h"
#include "cepch.h"

#include "CatEngine/Scene/Components/Physics/Rigidbody2D.h"
#include "CatEngine/Scene/Entity.h"
#include "CatEngine/Scene/Scene.h"

namespace CatEngine {

static b2BodyType
Rigidbody2DTypeToBox2DType(Rigidbody2DComponent::BodyType bodyType) {
  switch (bodyType) {
  case Rigidbody2DComponent::BodyType::Static:
    return b2_staticBody;
  case Rigidbody2DComponent::BodyType::Kinematic:
    return b2_kinematicBody;
  case Rigidbody2DComponent::BodyType::Dynamic:
    return b2_dynamicBody;
  }

  CE_API_ASSERT(false, "Unknown body type!");
  return b2_staticBody;
}

Physics2D &Physics2D::Get() {
  static Physics2D instance;
  return instance;
}

void Physics2D::Start(Scene *context) {
  m_Context = context;
  m_PhysicsWorld = new b2World({0.0f, -9.8f});
  m_PhysicsWorld->SetContactListener(&m_ContactListenter);

  auto view = m_Context->m_Registry.view<Rigidbody2DComponent>();
  for (auto e : view) {
    Entity entity = {e, m_Context};
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();

    // 1. Compute true absolute world transform recursively
    glm::mat4 worldTransform = entity.GetWorldTransform();
    glm::vec3 worldPos, worldRot, worldScale;
    Math::DecomposeTransform(worldTransform, worldPos, worldRot, worldScale);

    b2BodyDef bodyDef;
    bodyDef.type = Rigidbody2DTypeToBox2DType(rb2d.Type);
    bodyDef.position.Set(worldPos.x, worldPos.y); // Use true world position
    bodyDef.angle = worldRot.z;                   // Use true world rotation

    b2Body *body = m_PhysicsWorld->CreateBody(&bodyDef);
    body->SetFixedRotation(rb2d.FixedRotation);
    rb2d.RuntimeBody = body;

    if (entity.HasComponent<BoxCollider2DComponent>()) {
      auto &bc2d = entity.GetComponent<BoxCollider2DComponent>();

      b2PolygonShape boxShape;
      // Use decomposed worldScale instead of cached WorldScale
      boxShape.SetAsBox(worldScale.x * bc2d.Size.x, worldScale.y * bc2d.Size.y,
                        {bc2d.Offset.x, bc2d.Offset.y},
                        glm::radians(bc2d.Rotation));

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &boxShape;
      fixtureDef.density = bc2d.Density;
      fixtureDef.friction = bc2d.Friction;
      fixtureDef.restitution = bc2d.Restitution;
      fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
      fixtureDef.userData.pointer = static_cast<uintptr_t>(entity.GetUUID());
      body->CreateFixture(&fixtureDef);
    }

    if (entity.HasComponent<CircleCollider2DComponent>()) {
      auto &cc2d = entity.GetComponent<CircleCollider2DComponent>();

      b2CircleShape circleShape;
      circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
      // Use decomposed worldScale
      circleShape.m_radius = worldScale.x * cc2d.Radius;

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &circleShape;
      fixtureDef.density = cc2d.Density;
      fixtureDef.friction = cc2d.Friction;
      fixtureDef.restitution = cc2d.Restitution;
      fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
      fixtureDef.userData.pointer = static_cast<uintptr_t>(entity.GetUUID());
      body->CreateFixture(&fixtureDef);
    }
  }
}

void Physics2D::Update(Time ts) {
  SyncColliders();

  const int32_t velocityIterations = 6;
  const int32_t positionIterations = 6;

  m_PhysicsWorld->Step(ts.deltaTime(), velocityIterations, positionIterations);

  // Retrieve transform from Box2D
  auto view = m_Context->m_Registry.view<Rigidbody2DComponent>();
  for (auto e : view) {
    Entity entity{e, m_Context};
    auto &transform = entity.GetComponent<TransformComponent>();
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
    auto &rc = entity.GetComponent<RelationshipComponent>();

    b2Body *body = (b2Body *)rb2d.RuntimeBody;
    if (!body)
      continue;

    const auto &position = body->GetPosition();
    float angle = body->GetAngle();

    if (rc.Parent != 0) {
      // --- CHILD ENTITY ---
      Entity parent = m_Context->GetEntityByUUID(rc.Parent);
      if (parent) {
        // 1. Build the body's world matrix from Box2D data
        glm::mat4 bodyWorldMatrix =
            glm::translate(glm::mat4(1.0f), {position.x, position.y, 0.0f}) *
            glm::rotate(glm::mat4(1.0f), angle, {0.0f, 0.0f, 1.0f}) *
            glm::scale(glm::mat4(1.0f), transform.LocalScale);

        // 2. Convert world matrix to local space relative to the parent
        glm::mat4 parentWorldMatrix = parent.GetWorldTransform();
        glm::mat4 localMatrix =
            glm::inverse(parentWorldMatrix) * bodyWorldMatrix;

        // 3. Decompose local matrix
        glm::vec3 translation, rotation, scale;
        Math::DecomposeTransform(localMatrix, translation, rotation, scale);

        // 4. Assign back using your LocalPosition.z 2D angle convention
        transform.LocalPosition.x = translation.x;
        transform.LocalPosition.y = translation.y;
        transform.LocalPosition.z = rotation.z; // Local 2D rotation angle
        transform.LocalScale = scale;
      }
    } else {
      // --- ROOT ENTITY ---
      transform.LocalPosition.x = position.x;
      transform.LocalPosition.y = position.y;
      transform.LocalPosition.z = angle;
    }
  }
}

void Physics2D::Stop() {
  if (m_PhysicsWorld) {
    m_PhysicsWorld->SetContactListener(nullptr);
    delete m_PhysicsWorld;
    m_PhysicsWorld = nullptr;
  }
}

void Physics2D::ApplyForce(const Entity &entity, const glm::vec2 &impulse,
                           const glm::vec2 &point, bool wake) {
  if (!entity.HasComponent<Rigidbody2DComponent>())
    return;

  auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2Body *body = (b2Body *)rb2d.RuntimeBody;
  body->ApplyForce(b2Vec2(impulse.x, impulse.y), b2Vec2(point.x, point.y),
                   wake);
}

void Physics2D::ApplyForceToCenter(const Entity &entity,
                                   const glm::vec2 &impulse, bool wake) {
  if (!entity.HasComponent<Rigidbody2DComponent>())
    return;

  auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2Body *body = (b2Body *)rb2d.RuntimeBody;
  body->ApplyForceToCenter(b2Vec2(impulse.x, impulse.y), wake);
}

void Physics2D::ApplyLinearImpulse(const Entity &entity,
                                   const glm::vec2 &impulse,
                                   const glm::vec2 &point, bool wake) {
  if (!entity.HasComponent<Rigidbody2DComponent>())
    return;

  auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2Body *body = (b2Body *)rb2d.RuntimeBody;
  body->ApplyLinearImpulse(b2Vec2(impulse.x, impulse.y),
                           b2Vec2(point.x, point.y), wake);
}

void Physics2D::ApplyLinearImpulseToCenter(const Entity &entity,
                                           const glm::vec2 &impulse,
                                           bool wake) {
  if (!entity.HasComponent<Rigidbody2DComponent>())
    return;

  auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
  b2Body *body = (b2Body *)rb2d.RuntimeBody;
  body->ApplyLinearImpulseToCenter(b2Vec2(impulse.x, impulse.y), wake);
}

void Physics2D::SyncColliders() {
  if (!m_PhysicsWorld || !m_Context)
    return;

  // --- 1. Sync BoxCollider2D components ---
  auto boxView =
      m_Context->m_Registry.view<TransformComponent, Rigidbody2DComponent,
                                 BoxCollider2DComponent>();
  for (auto e : boxView) {
    Entity entity{e, m_Context};
    auto &transform = entity.GetComponent<TransformComponent>();
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
    auto &bc2d = entity.GetComponent<BoxCollider2DComponent>();

    if (!rb2d.RuntimeBody || !bc2d.RuntimeFixture)
      continue;
    if (!bc2d.IsDirty)
      continue; // Skip if nothing changed

    b2Fixture *fixture = (b2Fixture *)bc2d.RuntimeFixture;
    b2Body *body = (b2Body *)rb2d.RuntimeBody;

    // Update shape dimensions and offset
    b2PolygonShape *shape = (b2PolygonShape *)fixture->GetShape();

    // Get absolute world scale to scale the collider correctly with the
    // transform
    glm::mat4 worldTransform = entity.GetWorldTransform();
    glm::vec3 worldPos, worldRot, worldScale;
    Math::DecomposeTransform(worldTransform, worldPos, worldRot, worldScale);

    shape->SetAsBox(
        worldScale.x * bc2d.Size.x * 0.5f, worldScale.y * bc2d.Size.y * 0.5f,
        b2Vec2(bc2d.Offset.x, bc2d.Offset.y), glm::radians(bc2d.Rotation));

    // Update material properties
    fixture->SetDensity(bc2d.Density);
    fixture->SetFriction(bc2d.Friction);
    fixture->SetRestitution(bc2d.Restitution);

    // Mass data must be reset whenever density or shape size changes
    body->ResetMassData();

    // Clear the dirty flag
    bc2d.IsDirty = false;
  }

  // --- 2. Sync CircleCollider2D components (repeat the same logic) ---
  auto circleView =
      m_Context->m_Registry.view<TransformComponent, Rigidbody2DComponent,
                                 CircleCollider2DComponent>();
  for (auto e : circleView) {
    Entity entity{e, m_Context};
    auto &transform = entity.GetComponent<TransformComponent>();
    auto &rb2d = entity.GetComponent<Rigidbody2DComponent>();
    auto &cc2d = entity.GetComponent<CircleCollider2DComponent>();

    if (!rb2d.RuntimeBody || !cc2d.RuntimeFixture)
      continue;
    if (!cc2d.IsDirty)
      continue;

    b2Fixture *fixture = (b2Fixture *)cc2d.RuntimeFixture;
    b2Body *body = (b2Body *)rb2d.RuntimeBody;

    b2CircleShape *shape = (b2CircleShape *)fixture->GetShape();

    glm::mat4 worldTransform = entity.GetWorldTransform();
    glm::vec3 worldPos, worldRot, worldScale;
    Math::DecomposeTransform(worldTransform, worldPos, worldRot, worldScale);

    shape->m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
    shape->m_radius =
        worldScale.x * cc2d.Radius; // Uniform scale assumption for circles

    fixture->SetDensity(cc2d.Density);
    fixture->SetFriction(cc2d.Friction);
    fixture->SetRestitution(cc2d.Restitution);

    body->ResetMassData();
    cc2d.IsDirty = false;
  }
}

} // namespace CatEngine
