// emulator/display/virtual_display.cpp — 80x25 character-mode virtual display
#include "../hardware.hpp"
#include <cstring>

namespace emulator
{

    void VirtualDisplay::clear()
    {
        _screen.assign(size_t(_cfg.rows) * _cfg.cols, ' ');
        _cx = _cy = 0;
    }

    void VirtualDisplay::put_char(char c)
    {
        if (_screen.empty())
            clear();
        if (c == '\n')
        {
            _cx = 0;
            ++_cy;
        }
        else if (c == '\r')
        {
            _cx = 0;
        }
        else
        {
            if (_cx >= _cfg.cols)
            {
                _cx = 0;
                ++_cy;
            }
            if (_cy >= _cfg.rows)
            {
                _screen.erase(0, _cfg.cols);
                _screen.resize(size_t(_cfg.rows) * _cfg.cols, ' ');
                --_cy;
            }
            _screen[size_t(_cy) * _cfg.cols + _cx++] = c;
        }
    }

    void VirtualDisplay::write(const std::string &s)
    {
        for (char c : s)
            put_char(c);
        // present to console (virtual display is the console for now)
        std::fputs(s.c_str(), stdout);
        std::fflush(stdout);
    }

    bool VirtualDisplay::self_test(std::string &detail)
    {
        detail = std::to_string(_cfg.cols) + "x" + std::to_string(_cfg.rows) + " + graphics " + std::to_string(_cfg.gfx_w) + "x" + std::to_string(_cfg.gfx_h);
        // real test: render a frame into the buffer and verify a cell
        clear();
        write("M"); // goes to buffer and console
        return _screen[0] == 'M';
    }

} // namespace emulator
