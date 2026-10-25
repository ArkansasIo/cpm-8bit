// emulator/machine.cpp — Machine singleton + snapshot save/load (Phase 1/16)
#include "hardware.hpp"
#include <fstream>

namespace emulator
{

    Machine &Machine::instance()
    {
        static Machine m;
        return m;
    }

    bool Machine::save_state(const std::string &path) const
    {
        std::ofstream out(path, std::ios::binary);
        if (!out)
            return false;
        // versioned snapshot header: magic + format version + config
        const char magic[8] = {'M', 'S', 'T', 'S', 'S', 'N', 'P', '1'};
        out.write(magic, 8);
        out.write(reinterpret_cast<const char *>(&_cfg.ram_mb), sizeof(_cfg.ram_mb));
        out.write(reinterpret_cast<const char *>(&_cfg.disk_gb), sizeof(_cfg.disk_gb));
        out.write(reinterpret_cast<const char *>(&_cfg.cols), sizeof(_cfg.cols));
        out.write(reinterpret_cast<const char *>(&_cfg.rows), sizeof(_cfg.rows));
        out.write(reinterpret_cast<const char *>(&_cfg.gfx_w), sizeof(_cfg.gfx_w));
        out.write(reinterpret_cast<const char *>(&_cfg.gfx_h), sizeof(_cfg.gfx_h));
        const char netc = _cfg.network ? 1 : 0;
        out.write(&netc, 1);
        return out.good();
    }

    bool Machine::load_state(const std::string &path)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return false;
        char magic[8];
        in.read(magic, 8);
        if (std::string(magic, 8) != "MSTSSNP1")
            return false; // reject incompatible safely
        in.read(reinterpret_cast<char *>(&_cfg.ram_mb), sizeof(_cfg.ram_mb));
        in.read(reinterpret_cast<char *>(&_cfg.disk_gb), sizeof(_cfg.disk_gb));
        in.read(reinterpret_cast<char *>(&_cfg.cols), sizeof(_cfg.cols));
        in.read(reinterpret_cast<char *>(&_cfg.rows), sizeof(_cfg.rows));
        in.read(reinterpret_cast<char *>(&_cfg.gfx_w), sizeof(_cfg.gfx_w));
        in.read(reinterpret_cast<char *>(&_cfg.gfx_h), sizeof(_cfg.gfx_h));
        char netc = 0;
        in.read(&netc, 1);
        _cfg.network = netc != 0;
        return in.good();
    }

} // namespace emulator
