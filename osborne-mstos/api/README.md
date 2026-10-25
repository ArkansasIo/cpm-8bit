# MSTOS 2.2 API

Stable C++23 boundary between applications and MSTOS services.

```cpp
#include <mstos/api.hpp>

using namespace MSTOS;

int main() {
    File f = OpenFile("C:/notes.txt", FileMode::Write);
    WriteFile(f, "hello\n", 6);
    CloseFile(f);

    Window w = CreateWindow(640, 480, "Hello");
    DrawText("MSTOS 2.2", 10, 10);
    Refresh();

    Sleep(1000);
}
```

Categories: Files, Processes, Windows, Graphics, Input, Audio, Networking,
Storage, System information, Timers, IPC.

The implementation is provided by MSTOS services (`mstos/services`); nothing
in `api/include` depends on the emulator, the Linux kernel, or a filesystem.
