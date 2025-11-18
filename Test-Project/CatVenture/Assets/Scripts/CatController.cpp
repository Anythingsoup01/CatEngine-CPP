#include "CatController.h"
namespace CatRuntime
{

    void CatController::Start()
    {
        // Code placed here runs immediately after pressing play
    }

    void CatController::Update(float ts)
    {
        // Code placed here runs every frame
    }

    CatController* CatController::Create(uint64_t entityID) { return new CatController(entityID); }

}