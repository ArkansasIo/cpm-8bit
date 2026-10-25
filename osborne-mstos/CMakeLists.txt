cmake_minimum_required(VERSION 3.20)
project(osborne-mstos CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mstos
    emulator/machine.cpp
    emulator/main.cpp
    emulator/memory/virtual_ram.cpp
    emulator/storage/virtual_disk.cpp
    emulator/display/virtual_display.cpp
    emulator/keyboard/virtual_keyboard.cpp
    emulator/network/virtual_network.cpp
    emulator/cpu/virtual_cpu.cpp
    mstos/boot/post.cpp
    mstos/boot/boot_manager.cpp
    mstos/core/mstos.cpp
    mstos/shell/shell.cpp
    mstos/compatibility/cpm.cpp
    mstos/recovery/recovery.cpp
    mstos/audio/audio_system.cpp
    gui/boot/boot_screen.cpp
    gui/desktop/desktop.cpp
    apps/terminal/terminal.cpp
    apps/systeminfo/systeminfo.cpp
)

target_include_directories(mstos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} api/include)

# full warnings for the debug pass
if(MSVC)
    target_compile_options(mstos PRIVATE /W4 /permissive- /w14242 /w14254 /w14263
        /w14265 /w14287 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549
        /w14555 /w14640 /w14826 /w14905 /w14906 /w14928)
else()
    target_compile_options(mstos PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(WIN32)
    target_link_libraries(mstos PRIVATE user32)
endif()
