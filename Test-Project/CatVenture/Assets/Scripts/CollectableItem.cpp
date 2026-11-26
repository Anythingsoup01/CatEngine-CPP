#include "CollectableItem.h"
#include "PlayerController.h"

#include <CatScriptCore/Scene/Commands.h>

namespace CatRuntime {

CollectableItem::CollectableItem() {}
CollectableItem::~CollectableItem() {}

void CollectableItem::Start() {}

void CollectableItem::OnTriggerEnter(Object other) {
  if (other.HasScript<PlayerController>()) {
    auto *playerController = other.GetScript<PlayerController>();
    playerController->ApplyHealth(m_HealthModification);

    Scene::Destroy(this->m_EntityID);
  }
}

void CollectableItem::OnTriggerExit(Object other) {

}

} // namespace CatRuntime
