// apps/terminal/terminal.cpp — Terminal app (Osborne-style)
// The in-shell terminal is the MSTOS shell; this file provides the
// standalone terminal application entry used by the desktop launcher.
#include "terminal.hpp"
#include "mstos/shell/shell.hpp"

namespace apps::terminal
{

    void launch()
    {
        // runs the MSTOS shell as the terminal application
        mstos::shell::run();
    }

} // namespace apps::terminal
