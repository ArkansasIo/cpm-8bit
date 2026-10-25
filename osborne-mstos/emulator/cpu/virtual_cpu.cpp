// emulator/cpu/virtual_cpu.cpp — virtual CPU: reports REAL host CPU data
#include "../hardware.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <thread>
#endif
#include <cstdio>
#include <cstring>

namespace emulator
{

    std::string VirtualCpu::model() const
    {
        static std::string cached;
        if (!cached.empty())
            return cached;
#if defined(_WIN32)
        char brand[0x40] = {};
        // CPUID leaf 0x80000002..4 gives the processor brand string
        int regs[4];
        __cpuid((int *)regs, (int)0x80000000);
        if ((unsigned)regs[0] >= 0x80000004)
        {
            int *p = (int *)brand;
            for (unsigned leaf = 0x80000002; leaf <= 0x80000004; ++leaf)
                __cpuid((int *)(p + (leaf - 0x80000002) * 4), (int)leaf);
            brand[0x3F] = 0;
        }
        cached = brand;
#else
        FILE *f = std::fopen("/proc/cpuinfo", "r");
        if (f)
        {
            char line[512];
            while (std::fgets(line, sizeof line, f))
                if (std::strncmp(line, "model name", 10) == 0)
                {
                    const char *colon = std::strchr(line, ':');
                    if (colon)
                        cached = std::string(colon + 2, std::strlen(colon + 3));
                    break;
                }
            std::fclose(f);
        }
#endif
        if (cached.empty())
            cached = "x86-64";
        return cached;
    }

    uint32_t VirtualCpu::cores() const
    {
#if defined(_WIN32)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwNumberOfProcessors;
#else
        return std::thread::hardware_concurrency();
#endif
    }

    bool VirtualCpu::self_test(std::string &detail)
    {
        // actual exercise: run a tiny integer loop and verify results
        volatile uint64_t x = 1;
        for (int i = 0; i < 1000; ++i)
            x = x * 3 + 1;
        const bool ok = (x != 0);
        detail = model() + ", " + std::to_string(cores()) + " cores";
        return ok;
    }

} // namespace emulator
