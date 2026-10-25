// emulator/storage/virtual_disk.cpp — file-backed virtual storage
#include "../hardware.hpp"
#include <cstdio>
#include <filesystem>
#include <vector>

namespace emulator
{

    bool VirtualDisk::attach_image(const std::string &path)
    {
        _image_path = path;
        std::error_code ec;
        if (!std::filesystem::exists(path, ec))
            return false;
        return true;
    }

    bool VirtualDisk::self_test(std::string &detail)
    {
        const uint32_t gb = emulator::Machine::instance().config().disk_gb;
        detail = std::to_string(gb) + " GB";
        try
        {
            // real I/O test: create a temp scratch file in the current directory,
            // write, read back, delete — proves the storage path works.
            const char *tmp = "osborne_disk_probe.tmp";
            {
                std::FILE *f = std::fopen(tmp, "wb");
                if (!f)
                {
                    detail += " FAULT (write)";
                    return false;
                }
                const unsigned char pat[512] = {1};
                std::fwrite(pat, 1, sizeof pat, f);
                std::fclose(f);
            }
            std::FILE *f = std::fopen(tmp, "rb");
            if (!f)
            {
                detail += " FAULT (open)";
                return false;
            }
            unsigned char back[512] = {};
            const bool ok = std::fread(back, 1, sizeof back, f) == sizeof back && back[0] == 1;
            std::fclose(f);
            std::remove(tmp);
            if (!ok)
            {
                detail += " FAULT (verify)";
                return false;
            }
            return true;
        }
        catch (...)
        {
            detail += " FAULT";
            return false;
        }
    }

} // namespace emulator
