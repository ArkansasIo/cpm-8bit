// emulator/hardware.hpp — virtual Osborne hardware registry (Phase 1)
//
// All virtual devices live here. POST (mstos/boot) interrogates THIS
// configuration; nothing is hard-coded in the boot path.
#pragma once
#include <cstdint>
#include <string>

namespace emulator
{

    struct HardwareConfig
    {
        // virtual RAM (MiB) — configurable, e.g. --ram 16384
        uint32_t ram_mb = 16384;
        // virtual storage (GiB) — configurable, e.g. --disk 512
        uint32_t disk_gb = 512;
        // display geometry
        uint32_t cols = 80, rows = 25;     // character mode
        uint32_t gfx_w = 640, gfx_h = 480; // graphics mode
        // virtual network
        bool network = true;
        // saved-state file (Phase 1 "when practical")
        std::string state_file = "osborne.state";
    };

    // A virtual device reports its own health (used by POST).
    class VirtualDevice
    {
    public:
        virtual ~VirtualDevice() = default;
        virtual const char *name() const = 0;
        virtual bool self_test(std::string &detail) = 0; // returns OK + detail string
    };

    // ---- Virtual CPU ----
    class VirtualCpu final : public VirtualDevice
    {
    public:
        explicit VirtualCpu(const HardwareConfig &c) : _cfg(c) {}
        const char *name() const override { return "CPU"; }
        bool self_test(std::string &detail) override;
        std::string model() const; // real host CPU model
        uint32_t cores() const;    // real host core count
    private:
        HardwareConfig _cfg;
    };

    // ---- Virtual RAM ----
    class VirtualRam final : public VirtualDevice
    {
    public:
        explicit VirtualRam(const HardwareConfig &c) : _cfg(c) {}
        const char *name() const override { return "MEMORY"; }
        bool self_test(std::string &detail) override; // reports configured MB + write/read test
        uint32_t megabytes() const { return _cfg.ram_mb; }

    private:
        HardwareConfig _cfg;
    };

    // ---- Virtual Storage ----
    class VirtualDisk final : public VirtualDevice
    {
    public:
        explicit VirtualDisk(const HardwareConfig &c) : _cfg(c) {}
        const char *name() const override { return "STORAGE"; }
        bool self_test(std::string &detail) override; // reports configured GB + image I/O test
        uint32_t gigabytes() const { return _cfg.disk_gb; }
        bool attach_image(const std::string &path); // file-backed virtual disk
    private:
        HardwareConfig _cfg;
        std::string _image_path;
    };

    // ---- Virtual Display ----
    class VirtualDisplay final : public VirtualDevice
    {
    public:
        explicit VirtualDisplay(const HardwareConfig &c) : _cfg(c) {}
        const char *name() const override { return "VIDEO"; }
        bool self_test(std::string &detail) override;
        void put_char(char c); // 80x25 character-mode emulation
        void write(const std::string &s);
        void clear();

    private:
        HardwareConfig _cfg;
        std::string    _screen;
        uint32_t       _cx = 0, _cy = 0;
    };
    class VirtualKeyboard final : public VirtualDevice
    {
    public:
        const char *name() const override { return "KEYBOARD"; }
        bool self_test(std::string &detail) override; // probes the console input device
        bool poll_key(char &c, bool &is_enter, bool &is_escape);
    };

    // ---- Virtual Network ----
    class VirtualNetwork final : public VirtualDevice
    {
    public:
        explicit VirtualNetwork(const HardwareConfig &c) : _cfg(c) {}
        const char *name() const override { return "NETWORK"; }
        bool self_test(std::string &detail) override;
        bool available() const { return _cfg.network; }

    private:
        HardwareConfig _cfg;
    };

    // The machine: owns all virtual devices.
    class Machine
    {
    public:
        static Machine &instance();

        void configure(const HardwareConfig &c) { _cfg = c; }
        HardwareConfig &config() { return _cfg; }

        VirtualCpu &cpu() { return _cpu; }
        VirtualRam &ram() { return _ram; }
        VirtualDisk &disk() { return _disk; }
        VirtualDisplay &display() { return _display; }
        VirtualKeyboard &keyboard() { return _keyboard; }
        VirtualNetwork &net() { return _net; }

        // Phase 1: save/load VM state when practical
        bool save_state(const std::string &path) const;
        bool load_state(const std::string &path);

    private:
        Machine() = default;
        HardwareConfig _cfg;
        VirtualCpu _cpu{_cfg};
        VirtualRam _ram{_cfg};
        VirtualDisk _disk{_cfg};
        VirtualDisplay _display{_cfg};
        VirtualKeyboard _keyboard{};
        VirtualNetwork _net{_cfg};
    };

} // namespace emulator
