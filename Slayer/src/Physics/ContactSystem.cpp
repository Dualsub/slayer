#include "Physics/ContactSystem.h"

#include "Scene/World.h"

namespace Slayer {

    void ContactSystem::FixedUpdate(Timespan dt, ComponentStore& store)
    {
        auto& pw = World::GetPhysicsWorld();
        pw.ResetContacts();

        store.ForTransitionTo<RigidBody, CollisionListener>([&](Entity e, RigidBody* body, CollisionListener* listener)
            {
                pw.RegisterContactListener(body->id);
            }
        );

        store.ForTransitionFrom<RigidBody, CollisionListener>([&](Entity e, RigidBody* body, CollisionListener* listener)
            {
                pw.UnregisterContactListener(body->id);
            }
        );

    }

}