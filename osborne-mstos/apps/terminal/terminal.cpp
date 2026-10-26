// apps/terminal/terminal.cpp — Terminal app (Osborne-style)
// The in-shell terminal is the osbex shell; this file provides the
// standalone terminal application entry used by the desktop launcher.
#include "terminal.hpp"
#include "osbex/shell/shell.hpp"

namespace apps::terminal
{

    void launch()
    {
        // runs the osbex shell as the terminal application
        osbex::shell::run();
    }

} // namespace apps::terminal
