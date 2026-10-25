// mstos/compatibility/cpm.hpp — CP/M compatibility layer interface (isolated)
#pragma once
#include <string>

namespace mstos::compat
{

    // virtual drive mapping
    //   A:  Osborne disk image
    //   B:  Osborne disk image
    //   C:  MSTOS filesystem
    //   D:  modern storage
    enum class Drive : char
    {
        A = 'A',
        B = 'B',
        C = 'C',
        D = 'D'
    };

    // shell command entry point (e.g. "cpm mount A: image.dsk")
    void command(const std::string &line);

    // boot the CP/M disk (boot manager option 4)
    void boot_cpm();

}
