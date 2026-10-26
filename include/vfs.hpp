// include/vfs.hpp — redesigned kernel: minimal OOP VFS layer
//
// Linux has struct file_operations; we have kernel::Filesystem (abstract)
// with mounted instances registered into the VFS table. The DOS API's file
// calls (open/read/write) can now route through this instead of touching
// hardware directly — the redesign decouples DOS 2.2 semantics from drivers.
#pragma once
#include "types.hpp"

namespace kernel
{
    // what a filesystem driver must provide (like file_operations)
    class Filesystem
    {
    public:
        virtual ~Filesystem() = default;
        virtual const char *name() const = 0;
        // max 4 mounted filesystems, 8-char names
        virtual bool read(u8 unit, u16 lba, u16 count, u8 *buf) = 0;
        virtual bool write(u8 unit, u16 lba, u16 count, const u8 *buf) = 0;
        virtual u16 sector_size() const { return 512; }
    };

    class Vfs
    {
    public:
        static Vfs &instance()
        {
            static Vfs v;
            return v;
        }

        bool mount(Filesystem &fs, u8 unit)
        {
            if (_count >= MaxMounts)
                return false;
            _mounts[_count].fs = &fs;
            _mounts[_count].unit = unit;
            ++_count;
            return true;
        }

        Filesystem *find(const char *fs_name) const
        {
            for (u8 i = 0; i < _count; ++i)
            {
                const char *n = _mounts[i].fs->name();
                u8 k = 0;
                bool same = true;
                while (n[k] || fs_name[k])
                {
                    if (n[k] != fs_name[k])
                    {
                        same = false;
                        break;
                    }
                    ++k;
                    if (k > 16)
                    {
                        same = false;
                        break;
                    }
                }
                if (same)
                    return _mounts[i].fs;
            }
            return nullptr;
        }

        bool read(const char *fs_name, u8 unit, u16 lba, u16 count, u8 *buf)
        {
            Filesystem *fs = find(fs_name);
            return fs && fs->read(unit, lba, count, buf);
        }

        bool write(const char *fs_name, u8 unit, u16 lba, u16 count, const u8 *buf)
        {
            Filesystem *fs = find(fs_name);
            return fs && fs->write(unit, lba, count, buf);
        }

        [[nodiscard]] u8 mount_count() const { return _count; }

    private:
        struct Mount
        {
            Filesystem *fs;
            u8 unit;
        };
        static constexpr u8 MaxMounts = 4;
        Mount _mounts[MaxMounts]{};
        u8 _count = 0;
    };

    // RAM disk "filesystem" — the default mount; honest 16-bit block device
    class RamFs final : public Filesystem
    {
    public:
        const char *name() const override { return "ramfs"; }
        bool read(u8, u16 lba, u16 count, u8 *buf) override
        {
            for (u16 i = 0; i < count; ++i)
            {
                const u32 off = (u32(lba + i) * sector_size()) % RamSize;
                for (u16 b = 0; b < sector_size(); ++b)
                    buf[i * sector_size() + b] = _ram[(off + b) % RamSize];
            }
            return true;
        }
        bool write(u8, u16 lba, u16 count, const u8 *buf) override
        {
            for (u16 i = 0; i < count; ++i)
            {
                const u32 off = (u32(lba + i) * sector_size()) % RamSize;
                for (u16 b = 0; b < sector_size(); ++b)
                    _ram[(off + b) % RamSize] = buf[i * sector_size() + b];
            }
            return true;
        }

    private:
        static constexpr u16 RamSize = 16384; // 16 KB ramdisk
        u8 _ram[RamSize]{};
    };
} // namespace kernel
