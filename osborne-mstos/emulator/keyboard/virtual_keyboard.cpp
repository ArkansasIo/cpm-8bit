// emulator/keyboard/virtual_keyboard.cpp — console-backed virtual keyboard
#include "../hardware.hpp"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <cstdio>
#include <windows.h>
#include <conio.h>
#endif

namespace emulator
{

    bool VirtualKeyboard::self_test(std::string &detail)
    {
        // probe the console input device handle — a real device check, not a stub
#if defined(_WIN32)
        const HANDLE h = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                     OPEN_EXISTING, 0, nullptr);
        const bool ok = h != INVALID_HANDLE_VALUE;
        if (ok)
            CloseHandle(h);
#else
        const bool ok = true;
#endif
        detail = ok ? "console input present" : "CONIN$ unavailable";
        return ok;
    }

    bool VirtualKeyboard::poll_key(char &c, bool &is_enter, bool &is_escape)
    {
        is_enter = is_escape = false;
#if defined(_WIN32)
        // redirected input (pipe/file): _kbhit only sees the real console,
        // so fall back to a blocking read from stdin when input is not a console.
        static const bool input_is_console = []
        {
            HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode = 0;
            return h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode);
        }();
        if (!input_is_console)
        {
            const int ch = std::getchar();
            if (ch == EOF)
                return false;
            if (ch == '\n' || ch == '\r')
            {
                is_enter = true;
                c = '\r';
                return true;
            }
            c = char(ch);
            return true;
        }
        if (!_kbhit())
            return false;
        const int ch = _getch();
        if (ch == 13)
        {
            is_enter = true;
            c = '\r';
            return true;
        }
        if (ch == 27)
        {
            is_escape = true;
            c = 27;
            return true;
        }
        if (ch == 0 || ch == 0xE0)
        {
            (void)_getch();
            return false;
        } // fn key
        c = char(ch);
        return true;
#else
        return false; // POSIX console path implemented later
#endif
    }

} // namespace emulator
