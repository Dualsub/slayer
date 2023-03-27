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

    template <typename Serializer>
    void Serializer(Serializer& serializer)
    {
        serializer.serialize("texturePath", texturePath);
    }
};
