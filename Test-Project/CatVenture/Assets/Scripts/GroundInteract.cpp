#include "GroundInteract.h"
#include "PlayerController.h"

namespace CatRuntime {

void GroundInteract::Start() {
  // Code placed here runs immediately after pressing play
}

void GroundInteract::Update(float ts) {
  // Code placed here runs every frame
}

void GroundInteract::OnCollisionEnter(Object other) {
  if (other.HasScript<PlayerController>()) {
    PlayerController *ps = other.GetScript<PlayerController>();
    ps->m_PlayerSpeed = 5000.0f;
  }
}

void GroundInteract::OnCollisionExit(Object other) {
  if (other.tag() == "Player") { // Player collided
  }
}

} // namespace CatRuntime
