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

if(WIN32)
    target_link_libraries(mstos PRIVATE user32)
endif()
