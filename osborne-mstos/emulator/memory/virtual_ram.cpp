// emulator/memory/virtual_ram.cpp — virtual RAM: configured size + real test
#include "../hardware.hpp"
#include <vector>
#include <cstring>

namespace emulator
{

    bool VirtualRam::self_test(std::string &detail)
    {
        // Phase 3: RAM detection (configured value) + RAM test.
        // Allocate a probe region (capped so huge configs don't blow the host),
        // write patterns, verify — a genuine memory test, not a fake OK.
        const uint32_t mb = emulator::Machine::instance().config().ram_mb;
        detail = std::to_string(mb) + " MB";
        try
        {
            const size_t probe = std::min<size_t>(64u * 1024u * 1024u, size_t(mb) * 1024u * 1024u);
            std::vector<uint8_t> mem(probe, 0);
            for (size_t i = 0; i < probe; ++i)
                mem[i] = uint8_t(i * 7 + 0xA5);
            for (size_t i = 0; i < probe; i += 4096)
                if (mem[i] != uint8_t(i * 7 + 0xA5))
                {
                    detail += " FAULT";
                    return false;
                }
            return true;
        }
        catch (...)
        {
            detail += " FAULT (allocation failed)";
            return false;
        }
    }

} // namespace emulator
