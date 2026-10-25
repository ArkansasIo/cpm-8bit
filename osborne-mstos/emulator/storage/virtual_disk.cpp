// emulator/storage/virtual_disk.cpp — file-backed virtual storage
#include "../hardware.hpp"
#include <filesystem>
#include <fstream>
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
            // real I/O test: create a temp scratch file, write, read back, delete
            const char *tmp = "osborne_disk_probe.tmp";
            {
                std::ofstream f(tmp, std::ios::binary);
                if (!f)
                {
                    detail += " FAULT (write)";
                    return false;
                }
                const unsigned char pat[512] = {1};
                f.write(reinterpret_cast<const char *>(pat), sizeof pat);
            }
            std::ifstream f(tmp, std::ios::binary);
            if (!f)
            {
                detail += " FAULT (open)";
                return false;
            }
            unsigned char back[512] = {};
            f.read(reinterpret_cast<char *>(back), sizeof back);
            const bool ok = static_cast<size_t>(f.gcount()) == sizeof back && back[0] == 1;
            f.close();
            std::filesystem::remove(tmp);
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
