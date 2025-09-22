#include "Logger.h"
#include <string>     // <-- for std::wstring
#include <shlwapi.h>  // PathRemoveFileSpecW
#pragma comment(lib, "Shlwapi.lib")

SimpleLogger g_log;
std::atomic<long> g_jitLoggedCount{ 0 };

static void DebugPrintLastError(const wchar_t* prefix, const wchar_t* path) {
    DWORD err = GetLastError();
    wchar_t buf[512];
    swprintf_s(buf, L"%s (path=\"%s\") failed, GetLastError=%lu (0x%08X)\r\n",
        prefix, path ? path : L"(null)", err, err);
    OutputDebugStringW(buf);
}

SimpleLogger::SimpleLogger() : _h(INVALID_HANDLE_VALUE) {
    InitializeSRWLock(&_lock);
}

SimpleLogger::~SimpleLogger() {
    if (_h != INVALID_HANDLE_VALUE) CloseHandle(_h);
}

void SimpleLogger::EnsureTrailingSlash(wchar_t* s) {
    size_t len = wcslen(s);
    if (len == 0) return;
    if (s[len - 1] != L'\\') StringCchCatW(s, MAX_PATH, L"\\");
}

static bool TryGetEnvDup(const wchar_t* name, std::wstring& out) {
    wchar_t* val = nullptr;
    size_t len = 0;
    if (_wdupenv_s(&val, &len, name) == 0 && val) {
        out.assign(val);
        free(val);
        while (!out.empty() && (out.back() == L' ' || out.back() == L'\\' || out.back() == L'/'))
            out.pop_back();
        return true;
    }
    return false;
}

static void EnsureDirRecursive(const wchar_t* dir) {
    wchar_t tmp[MAX_PATH];
    StringCchCopyW(tmp, _countof(tmp), dir);
    for (wchar_t* p = tmp; *p; ++p) {
        if (*p == L'/' || *p == L'\\') {
            wchar_t ch = *p; *p = 0;
            if (wcslen(tmp) >= 3) CreateDirectoryW(tmp, nullptr);
            *p = ch;
        }
    }
    CreateDirectoryW(tmp, nullptr);
}

void SimpleLogger::InitOnce() {
    if (_h != INVALID_HANDLE_VALUE) return;
    __debugbreak();
    wchar_t path[MAX_PATH] = { 0 };

    // 0) MINIPROFILER_LOG (full path override)
    {
        std::wstring custom;
        if (TryGetEnvDup(L"MINIPROFILER_LOG", custom) && !custom.empty()) {
            wcsncpy_s(path, _countof(path), custom.c_str(), _TRUNCATE);   // <-- fixed
        }
    }

    // 1) Azure App Service: %HOME%\LogFiles\Application\MiniProfiler.log
    if (path[0] == 0) {
        std::wstring home;
        if (TryGetEnvDup(L"HOME", home) && !home.empty()) {
            wchar_t dir[MAX_PATH] = { 0 };
            wcsncpy_s(dir, _countof(dir), home.c_str(), _TRUNCATE);       // <-- fixed
            EnsureTrailingSlash(dir); StringCchCatW(dir, _countof(dir), L"LogFiles\\");
            EnsureDirRecursive(dir);

            EnsureTrailingSlash(dir); StringCchCatW(dir, _countof(dir), L"Application\\");
            EnsureDirRecursive(dir);

            StringCchCopyW(path, _countof(path), dir);
            EnsureTrailingSlash(path);
            StringCchCatW(path, _countof(path), L"MiniProfiler.log");
        }
    }

    // 2) Fallback: %TEMP%\MiniProfiler.log
    if (path[0] == 0) {
        wchar_t tempDir[MAX_PATH] = { 0 };
        if (GetTempPathW(_countof(tempDir), tempDir) == 0) {
            StringCchCopyW(tempDir, _countof(tempDir), L"C:\\Temp\\");
        }
        StringCchCopyW(path, _countof(path), tempDir);
        EnsureTrailingSlash(path);
        StringCchCatW(path, _countof(path), L"MiniProfiler.log");
    }

    // 3) Last resort attempts: chosen path, then C:\MiniProfiler.log
    wchar_t attemptPaths[3][MAX_PATH] = { 0 };
    StringCchCopyW(attemptPaths[0], _countof(attemptPaths[0]), path);
    StringCchCopyW(attemptPaths[1], _countof(attemptPaths[1]), L"C:\\MiniProfiler.log");
    StringCchCopyW(attemptPaths[2], _countof(attemptPaths[2]), L"");

    HANDLE h = INVALID_HANDLE_VALUE;
    for (int i = 0; i < 3 && h == INVALID_HANDLE_VALUE; ++i) {
        if (attemptPaths[i][0] == 0) continue;

        wchar_t dir[MAX_PATH] = { 0 };
        StringCchCopyW(dir, _countof(dir), attemptPaths[i]);
        PathRemoveFileSpecW(dir);
        if (dir[0]) EnsureDirRecursive(dir);

        h = CreateFileW(attemptPaths[i],
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            DebugPrintLastError(L"CreateFileW", attemptPaths[i]);
        }
        else {
            StringCchCopyW(path, _countof(path), attemptPaths[i]);
        }
    }

    AcquireSRWLockExclusive(&_lock);
    _h = h;
    ReleaseSRWLockExclusive(&_lock);

    if (_h != INVALID_HANDLE_VALUE) {
        WriteLine(L"==== MiniProfiler started ====");
        WriteLine(L"Log path: %s", path);
    }
    else {
        OutputDebugStringW(L"MiniProfiler: FAILED to open any log file. See previous OutputDebugString for GetLastError.\r\n");
    }
}

void SimpleLogger::WriteLine(const wchar_t* fmt, ...) noexcept {
    if (_h == INVALID_HANDLE_VALUE) return;

    wchar_t msg[1024];
    va_list args;
    va_start(args, fmt);
    HRESULT hr = StringCchVPrintfW(msg, _countof(msg), fmt, args);
    va_end(args);
    if (FAILED(hr)) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t line[1400];
    if (SUCCEEDED(StringCchPrintfW(line, _countof(line),
        L"[%04u-%02u-%02u %02u:%02u:%02u.%03u] %s\r\n",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, msg))) {

        const DWORD bytes = (DWORD)(lstrlenW(line) * sizeof(wchar_t));
        AcquireSRWLockExclusive(&_lock);
        DWORD written = 0;
        if (!WriteFile(_h, line, bytes, &written, nullptr)) {
            DebugPrintLastError(L"WriteFile", L"(log handle)");
        }
        ReleaseSRWLockExclusive(&_lock);
    }
}
