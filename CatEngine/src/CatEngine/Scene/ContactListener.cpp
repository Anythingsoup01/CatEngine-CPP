#include "cepch.h"
#include "ContactListener.h"

#include "CatEngine/Scripting/ScriptEngine.h"

namespace CatEngine
{
    void ContactListener::BeginContact(b2Contact* contact)
    {
        const auto& handleA = static_cast<UUID>(contact->GetFixtureA()->GetUserData().pointer);
        const auto& handleB = static_cast<UUID>(contact->GetFixtureB()->GetUserData().pointer);

        ScriptEngine::DispatchCollisionEvent(handleA, handleB, CollisionType::Begin);
    }
    void ContactListener::EndContact(b2Contact* contact)
    {
        const auto& handleA = static_cast<UUID>(contact->GetFixtureA()->GetUserData().pointer);
        const auto& handleB = static_cast<UUID>(contact->GetFixtureB()->GetUserData().pointer);

        ScriptEngine::DispatchCollisionEvent(handleA, handleB, CollisionType::End);
    }
}
