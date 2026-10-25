// include/dos_api.hpp — MS-DOS 2.2 compatible API layer (INT 21h)
#pragma once
#include "types.hpp"
#include "vga.hpp"
#include "keyboard.hpp"
#include "pit.hpp"

namespace dos
{

    // MS-DOS 2.2 function codes (AH register, 8-bit)
    enum class Fn : u8
    {
        ReadCharEcho = 0x01,  // read char from stdin with echo -> AL
        WriteChar = 0x02,     // write char in DL to stdout
        WriteString = 0x09,   // write '$'-terminated string at DS:DX
        BufferedInput = 0x0A, // buffered line input
        SetIntVector = 0x25,  // AL=vector, DS:DX=handler
        GetTime = 0x2C,       // CH:CL:DH:DL = h:m:s:hundredths
        GetIntVector = 0x35,  // AL=vector -> ES:BX
        Terminate = 0x4C,     // exit with code AL
    };

    // File Control Block — DOS 2.2 style (drive byte + 8.3 name)
    struct Fcb
    {
        u8 drive;     // 0 = default, 1 = A:, ...
        char name[8]; // 8 chars, space padded
        char ext[3];  // 3 chars, space padded
        u16 current_block;
        u16 record_size;
        u32 file_size;
        u16 date; // DOS 2.2 packed date
        u16 time; // DOS 2.2 packed time
    };

    // Object-oriented file handle over an FCB
    class FileHandle
    {
    public:
        FileHandle() = default;
        explicit FileHandle(const char *name11) { parse_name(name11); }

        bool parse_name(const char *name11); // "NAME", "EXT" packed 8.3
        void open();                         // set up FCB, mark open
        void close();
        [[nodiscard]] bool is_open() const { return _open; }
        [[nodiscard]] u32 size() const { return _fcb.file_size; }
        [[nodiscard]] const Fcb &fcb() const { return _fcb; }

    private:
        Fcb _fcb{};
        bool _open = false;
    };

    // The DOS API subsystem: implements INT 21h semantics.
    class DosApi
    {
    public:
        static DosApi &instance()
        {
            static DosApi a;
            return a;
        }

        void init(hw::Vga &v, hw::Keyboard &k, hw::Pit &p)
        {
            _vga = &v;
            _kbd = &k;
            _pit = &p;
        }

        // dispatch on AH; args come in as 8-bit registers
        void dispatch(u8 ah, u8 al, u8 dl, u8 dh, u8 &ret_al, u8 &ret_ah);

        // direct C++ side of the API
        void put_char(char c);
        void write_str(const char *s);
        char read_char();
        void get_time(u8 &h, u8 &m, u8 &s);
        void exit(u8 code);

        // C interface used by the int21 assembly router
        void set_int_vector(u8 vec, u16 seg, u16 off);
        void get_int_vector(u8 vec, u16 &seg, u16 &off);

        [[nodiscard]] u8 exit_code() const { return _exit_code; }
        [[nodiscard]] bool exited() const { return _exited; }

    private:
        DosApi() = default;
        hw::Vga *_vga = nullptr;
        hw::Keyboard *_kbd = nullptr;
        hw::Pit *_pit = nullptr;
        u8 _exit_code = 0;
        bool _exited = false;

        u16 _vec_seg[256]{}; // interrupt vector table shadow
        u16 _vec_off[256]{};
    };

    // IRQ plumbing (implemented in kernel.cpp)
    extern "C" void hw_irq_handler();
    extern "C" void dos_int21_handler();

} // namespace dos
