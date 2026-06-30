#include "util/PlatformUtils.h"
#include "util/Logger.h"

#include <QFileInfo>

#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")

namespace gpm {

void PlatformUtils::enableDpiAwareness() {
    // Per-Monitor V2（Windows 10 1703+）
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        using Fn = BOOL (WINAPI*)(DPI_AWARENESS_CONTEXT);
        if (auto setCtx = reinterpret_cast<Fn>(GetProcAddress(user32, "SetProcessDpiAwarenessContext"))) {
            setCtx(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }
}

static QString ws2q(const wchar_t* s) {
    return QString::fromWCharArray(s ? s : L"");
}

quint32 PlatformUtils::foregroundProcessId() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return 0;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return static_cast<quint32>(pid);
}

QString PlatformUtils::foregroundProcessPath() {
    quint32 pid = foregroundProcessId();
    if (!pid) return {};
    return processPathById(pid);
}

QString PlatformUtils::foregroundProcessName() {
    const QString p = foregroundProcessPath();
    if (p.isEmpty()) return {};
    return QFileInfo(p).fileName();
}

QString PlatformUtils::foregroundWindowTitle() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return {};
    wchar_t title[512] = {0};
    GetWindowTextW(hwnd, title, 512);
    return ws2q(title);
}

QString PlatformUtils::processPathById(quint32 pid) {
    if (!pid) return {};
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return {};
    wchar_t path[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    BOOL ok = QueryFullProcessImageNameW(h, 0, path, &size);
    CloseHandle(h);
    return ok ? ws2q(path) : QString();
}

QString PlatformUtils::processNameById(quint32 pid) {
    const QString p = processPathById(pid);
    if (p.isEmpty()) return {};
    return QFileInfo(p).fileName();
}

bool PlatformUtils::isProcessRunning(quint32 pid) {
    if (!pid) return false;
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return false;
    DWORD code = 0;
    BOOL ok = GetExitCodeProcess(h, &code);
    CloseHandle(h);
    return ok && code == STILL_ACTIVE;
}

quint32 PlatformUtils::findProcessByName(const QString& name) {
    if (name.isEmpty()) return 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    quint32 found = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (QString::fromWCharArray(pe.szExeFile).compare(name, Qt::CaseInsensitive) == 0) {
                found = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

bool PlatformUtils::isRunAsAdmin() {
    BOOL isMember = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = nullptr;
    if (AllocateAndInitializeSid(&NtAuthority, 2,
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(nullptr, adminGroup, &isMember);
        FreeSid(adminGroup);
    }
    return isMember == TRUE;
}

} // namespace gpm
