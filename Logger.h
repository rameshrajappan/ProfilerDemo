#pragma once
#include <windows.h>
#include <strsafe.h>
#include <atomic>

class SimpleLogger {
public:
    SimpleLogger();
    ~SimpleLogger();

    void InitOnce();
    void WriteLine(const wchar_t* fmt, ...) noexcept;

private:
    HANDLE  _h;
    SRWLOCK _lock;

    void EnsureTrailingSlash(wchar_t* s);
};

// Global logger instance (defined in Logger.cpp)
extern SimpleLogger g_log;

// Optional: global counters for throttling JIT logs
extern std::atomic<long> g_jitLoggedCount;
constexpr long kMaxJitLogs = 200;
