#include "application.hpp"
#include <filesystem>

/* sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules */

int main(int, char **)
{
    Application app;
    if (!app.Initialize())
    {
        return 1;
    }

    // app.testCommandQueue();

    app.MainLoop();

    while (app.IsRunning())
    {
        app.MainLoop();
    }

    app.Terminate();

    return 0;
}