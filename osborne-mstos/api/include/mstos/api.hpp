// api/include/mstos/api.hpp — the stable MSTOS 2.2 C++23 OS API
//
// This is the boundary between applications and MSTOS services.
// Nothing in this header leaks emulator, kernel or filesystem internals.
#pragma once
#include <cstdint>
#include <cstddef>

namespace MSTOS
{

    // ---------- files ----------
    enum class FileMode : uint8_t
    {
        Read,
        Write,
        ReadWrite,
        Append
    };

    struct File
    {
        void *handle = nullptr;
        bool ok = false;
        uint64_t size = 0;
    };

    File OpenFile(const char *path, FileMode mode = FileMode::Read);
    void CloseFile(File f);
    bool ReadFile(File f, void *buf, uint64_t bytes, uint64_t *read_out);
    bool WriteFile(File f, const void *buf, uint64_t bytes);
    bool DeleteFile(const char *path);
    uint64_t FileSize(const char *path);

    // ---------- processes ----------
    struct Process
    {
        void *handle = nullptr;
        bool ok = false;
        int32_t pid = -1;
    };

    Process CreateProcess(const char *program, const char *args = nullptr);
    bool WaitForProcess(Process p, int32_t *exit_code);
    void TerminateProcess(Process p, int32_t code);

    // ---------- windows / graphics ----------
    struct Window
    {
        void *handle = nullptr;
        bool ok = false;
    };

    Window CreateWindow(int width, int height, const char *title);
    void DestroyWindow(Window w);
    void DrawText(const char *text, int x, int y);
    void SetColor(uint8_t fg, uint8_t bg);
    void ClearScreen();
    void Refresh();

    // ---------- input ----------
    enum class Key : uint8_t
    {
        None,
        Enter,
        Escape,
        Up,
        Down,
        Left,
        Right,
        Char
    };
    struct KeyEvent
    {
        Key key;
        char character;
        bool pressed;
    };
    bool PollKey(KeyEvent *out);

    // ---------- audio ----------
    bool PlayBeep(uint32_t frequency_hz, uint32_t duration_ms);

    // ---------- networking ----------
    bool NetAvailable();
    bool HttpGet(const char *url, char *buf, size_t buf_size);

    // ---------- storage ----------
    struct StorageInfo
    {
        const char *drive;
        uint64_t total_bytes;
        uint64_t free_bytes;
    };
    bool QueryStorage(StorageInfo *out, size_t max_entries, size_t *count_out);

    // ---------- system information ----------
    struct SystemInfo
    {
        const char *os_name; // "MSTOS 2.2"
        const char *machine; // "Osborne (virtual)"
        const char *cpu_model;
        uint32_t cpu_cores;
        uint64_t ram_mb;
        uint64_t disk_gb;
        uint32_t screen_cols;
        uint32_t screen_rows;
    };
    SystemInfo GetSystemInfo();

    // ---------- timers ----------
    void Sleep(uint64_t milliseconds);
    uint64_t Ticks(); // monotonic milliseconds

    // ---------- IPC ----------
    bool IpcSend(const char *channel, const void *data, size_t bytes);
    bool IpcRecv(const char *channel, void *buf, size_t buf_size, size_t *received);

} // namespace MSTOS
