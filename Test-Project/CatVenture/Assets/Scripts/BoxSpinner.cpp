#include "BoxSpinner.h"
namespace CatRuntime {

static Transform s_Transform;

void BoxSpinner::Start() {
  // Code placed here runs immediately after pressing play
  std::cout << m_Control << "\n";
}

void BoxSpinner::Update(float ts) {
  // Code placed here runs every frame
  float curr_z_rot = transform().rotation().z();

  if (curr_z_rot >= 360.0f)
    curr_z_rot -= 360.0f;

  curr_z_rot += 1.0 * ts;

  transform().rotation().z() = curr_z_rot;
  transform().position().x() = 1;
}

} // namespace CatRuntime
