# osbex 2.2 API

Stable C++23 boundary between applications and osbex services.

```cpp
#include <osbex/api.hpp>

using namespace osbex;

int main() {
    File f = OpenFile("C:/notes.txt", FileMode::Write);
    WriteFile(f, "hello\n", 6);
    CloseFile(f);

    Window w = CreateWindow(640, 480, "Hello");
    DrawText("osbex 2.2", 10, 10);
    Refresh();

    Sleep(1000);
}
```

Categories: Files, Processes, Windows, Graphics, Input, Audio, Networking,
Storage, System information, Timers, IPC.

The implementation is provided by osbex services (`osbex/services`); nothing
in `api/include` depends on the emulator, the OS Prototype kernel, or a filesystem.
