#include "core/application.hpp"
#include "colorSorter/colorSorter.hpp"
#include <filesystem>

/* sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules */

int main(int, char **)
{
    // Application app;
    // if (!app.Initialize())
    // {
    //     return 1;
    // }
    std::string fullPath = std::filesystem::current_path().string() + "/resources/testSort.png";
    ImageData imageData(fullPath);

    // app.testCommandQueue();

    // app.MainLoop();

    // while (app.IsRunning())
    // {
    //     app.MainLoop();
    // }

    // app.Terminate();

    return 0;
}