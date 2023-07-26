#include <iostream>
#include "Core/Log.h"
#include "Core/Main.h"
#include "Core/Engine.h"
#include "Core/Application.h"

extern Slayer::Application* RegisterApplication(int argc, const char** argv);

int main(int argc, const char** argv)
{
    Slayer::Engine* engine = new Slayer::Engine();
    Slayer::Application* app = RegisterApplication(argc, argv);

    engine->RunMainLoop(app);

    delete app;
    delete engine;

    return 0;
}