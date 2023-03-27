#define BOOST_TEST_MODULE test module name
#include <boost/test/included/unit_test.hpp>
#include <string> 
#include "Scene/ComponentStore.h"

struct Position
{
    float x;
    float y;
};

struct Velocity
{
    float x;
    float y;
};

struct Renderable
{
    std::string texturePath;
};

BOOST_AUTO_TEST_CASE(AddComponent_Test)
{
    Slayer::ComponentStore ecs;

    ecs.RegisterComponent<Position>();

    Slayer::Entity entity = ecs.CreateEntity();

    Position position = { 10.0f, 20.0f };

    ecs.AddComponent(entity, position);

    Position* pos = ecs.GetComponent<Position>(entity);

    BOOST_TEST(pos->x == position.x);
    BOOST_TEST(pos->y == position.y);
}

BOOST_AUTO_TEST_CASE(MultiComponent_Test)
{
    Slayer::ComponentStore ecs;

    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Renderable>();

    Slayer::Entity entity = ecs.CreateEntity();

    Position position = { 10.0f, 20.0f };
    Velocity velocity = { 30.0f, 40.0f };
    Renderable renderable = { "texture.png" };

    ecs.AddComponent(entity, position);
    ecs.AddComponent(entity, velocity);
    ecs.AddComponent(entity, renderable);

    std::vector<Slayer::Entity> entities = ecs.GetEntities<Position, Velocity, Renderable>();

    BOOST_TEST(entities.size() == 1);

    Position* pos = ecs.GetComponent<Position>(entities[0]);
    Velocity* vel = ecs.GetComponent<Velocity>(entities[0]);
    Renderable* ren = ecs.GetComponent<Renderable>(entities[0]);

    BOOST_TEST(pos->x == position.x);
    BOOST_TEST(pos->y == position.y);
    BOOST_TEST(vel->x == velocity.x);
    BOOST_TEST(vel->y == velocity.y);
    BOOST_TEST(ren->texturePath == renderable.texturePath);
}

BOOST_AUTO_TEST_CASE(DestroyEntity_Test)
{
    Slayer::ComponentStore ecs;

    ecs.RegisterComponent<Position>();

    Slayer::Entity entity = ecs.CreateEntity();

    Position position = { 10.0f, 20.0f };

    ecs.AddComponent(entity, position);

    Position* pos = ecs.GetComponent<Position>(entity);

    BOOST_TEST(pos->x == position.x);
    BOOST_TEST(pos->y == position.y);

    ecs.DestroyEntity(entity);

    BOOST_TEST(!ecs.IsValid(entity));
}

BOOST_AUTO_TEST_CASE(BigNumberEntities_Test)
{
    Slayer::ComponentStore ecs;

    ecs.RegisterComponent<Position>();
    ecs.RegisterComponent<Velocity>();
    ecs.RegisterComponent<Renderable>();

    const int numEntities = MAX_ENTITIES;

    std::vector<Slayer::Entity> entities;

    for (int i = 0; i < numEntities; ++i)
    {
        entities.push_back(ecs.CreateEntity());

        Position position = { 10.0f, 20.0f };
        Velocity velocity = { 30.0f, 40.0f };
        Renderable renderable = { "texture.png" };

        ecs.AddComponent(entities[i], position);
        ecs.AddComponent(entities[i], velocity);
        ecs.AddComponent(entities[i], renderable);
    }

    std::vector<Slayer::Entity> entitiesWithComponents = ecs.GetEntities<Position, Velocity, Renderable>();

    BOOST_TEST(entitiesWithComponents.size() == numEntities);
}