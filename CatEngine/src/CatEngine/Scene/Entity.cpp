#include "cepch.h" // IWYU pragma: keep

#include "Entity.h"

namespace CatEngine {
Entity::Entity(entt::entity handle, Scene *scene)
    : m_EntityHandle(handle), m_Scene(scene) {}

} // namespace CatEngine
