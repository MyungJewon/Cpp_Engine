#include "app/DemoApp.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int, char**) {
#endif
    DemoApp app(800, 600, "Cpp_Engine — Demo");
    app.Run();
    return 0;
}
