#include "application.hpp"

/* sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules */

int main(int, char **)
{

    Application app;
    app.Initialize();

    app.testCommandQueue();

    app.MainLoop();

    while (app.IsRunning())
    {
        app.MainLoop();
    }

    app.Terminate();

    return 0;
}