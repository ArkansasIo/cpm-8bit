// mstos/shell/shell.cpp — MSTOS 2.2 shell (Osborne-style, C: drive, CP/M verbs)
#include "shell.hpp"
#include "emulator/hardware.hpp"
#include "mstos/compatibility/cpm.hpp"
#include "mstos/core/mstos.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>

namespace mstos::shell
{

    static std::string now_hhmmss()
    {
        using clock = std::chrono::system_clock;
        const std::time_t t = clock::to_time_t(clock::now());
        char buf[16];
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::strftime(buf, sizeof buf, "%H:%M:%S", &tm);
        return buf;
    }

    [[noreturn]] void run()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        auto &kbd = Machine::instance().keyboard();

        disp.write("MSTOS 2.2 shell — type HELP for commands.\n\n");

        std::string line;
        while (true)
        {
            disp.write("C:\\> ");
            line.clear();

            // line input via virtual keyboard
            while (true)
            {
                char c;
                bool enter, esc;
                if (!kbd.poll_key(c, enter, esc))
                    continue;
                if (enter)
                {
                    disp.write("\n");
                    break;
                }
                if (c == '\b')
                {
                    if (!line.empty())
                    {
                        line.pop_back();
                        disp.write("\b \b");
                    }
                    continue;
                }
                if (c >= 32 && c < 127)
                {
                    line += c;
                    disp.write(std::string(1, c));
                }
            }

            // dispatch
            if (line == "help" || line == "HELP" || line == "?")
            {
                disp.write(
                    "VER      - MSTOS 2.2 version and machine info\n"
                    "TIME     - current time\n"
                    "DIR      - list current directory (C:)\n"
                    "SYSINFO  - virtual hardware summary\n"
                    "CPM      - CP/M compatibility: A: B: images, C: MSTOS, D: modern\n"
                    "SAVE     - save virtual machine state\n"
                    "LOAD     - load virtual machine state\n"
                    "EXIT     - power off\n");
            }
            else if (line == "ver" || line == "VER")
            {
                disp.write("MSTOS 2.2 on Osborne (virtual) — heritage: Osborne 1, platform: Linux x86-64\n");
            }
            else if (line == "time" || line == "TIME")
            {
                disp.write(now_hhmmss() + "\n");
            }
            else if (line == "dir" || line == "DIR")
            {
                for (const auto &e : std::filesystem::directory_iterator("."))
                    disp.write("  " + e.path().filename().string() + "\n");
            }
            else if (line == "sysinfo" || line == "SYSINFO")
            {
                auto &m = Machine::instance();
                disp.write("CPU:     " + m.cpu().model() + " (" + std::to_string(m.cpu().cores()) + " cores)\n");
                disp.write("MEMORY:  " + std::to_string(m.ram().megabytes()) + " MB\n");
                disp.write("STORAGE: " + std::to_string(m.disk().gigabytes()) + " GB\n");
                disp.write("VIDEO:   80x25 text + 640x480 graphics\n");
                disp.write("NETWORK: " + std::string(m.net().available() ? "available" : "disabled") + "\n");
            }
            else if (line.rfind("cpm", 0) == 0)
            {
                compat::command(line);
            }
            else if (line == "save" || line == "SAVE")
            {
                disp.write(Machine::instance().save_state("osborne.state")
                               ? "VM state saved.\n"
                               : "save failed.\n");
            }
            else if (line == "load" || line == "LOAD")
            {
                disp.write(Machine::instance().load_state("osborne.state")
                               ? "VM state loaded.\n"
                               : "load failed.\n");
            }
            else if (line == "exit" || line == "EXIT")
            {
                disp.write("Powering off the Osborne...\n");
                std::exit(0);
            }
            else if (!line.empty())
            {
                disp.write("Bad command or file name\n");
            }
        }
    }

} // namespace mstos::shell
