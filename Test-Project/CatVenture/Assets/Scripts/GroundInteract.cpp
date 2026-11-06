#include "GroundInteract.h"
namespace CatRuntime {


void GroundInteract::Start() {
  // Code placed here runs immediately after pressing play
}

void GroundInteract::Update(float ts) {
  // Code placed here runs every frame
}

void GroundInteract::OnCollisionEnter(Object other) {
  if (other.tag() == "Player") { // Player collided
  }
}

void GroundInteract::OnCollisionExit(Object other) {
  if (other.tag() == "Player") { // Player collided
  }

}

} // namespace CatRuntime
