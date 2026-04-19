#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdbool.h>

// 语言 ID
#define LANG_ZH 0
#define LANG_EN 1
static int g_lang = LANG_ZH;   // 默认中文，启动时根据系统/参数修改

// 多语言字符串表
static const TCHAR* g_msg[][2] = {
    // 0: 标题
    { _T("====== Windows 运行库修复工具 ======"), _T("====== Windows Runtime Repair Tool ======") },
    // 1: 版本行
    { _T("               v1.0"), _T("               v1.0") },
    // 2: 分隔线
    { _T("===================================="), _T("====================================") },
    // 3: 系统版本信息前缀
    { _T("系统版本："), _T("System version: ") },
    // 4: 正在检测
    { _T("正在检测缺失的运行库......"), _T("Checking for missing runtimes......") },
    // 5: 没有缺失
    { _T("没有检测到缺失的运行库"), _T("No missing runtimes detected") },
    // 6: 缺失的运行库列表头
    { _T("缺失的运行库："), _T("Missing runtimes:") },
    // 7: 提示 v8/v9 总会安装
    { _T("\n因技术原因，无法准确检测是否安装了v8/9版本。所以无论如何都会被安装"), _T("\nFor technical reasons, v8/v9 detection is not accurate, they will always be installed.") },
    // 8: 开始安装提示
    { _T("按 Enter 键开始安装"), _T("Press Enter to start installation") },
    // 9: 已安装列表头
    { _T("\n已安装以下库："), _T("\nInstalled runtimes:") },
    // 10: 退出提示
    { _T("按 Enter 键退出"), _T("Press Enter to exit") },
    // 11: 继续提示（通用）
    { _T("按 Enter 键继续..."), _T("Press Enter to continue...") },
    // 12: 正在安装 X
    { _T("正在安装 %s"), _T("Installing %s") },
    // 13: 启动进程失败
    { _T("启动进程失败，错误码: %lu"), _T("Failed to start process, error code: %lu") },
    // 14: 需要 Win7+
    { _T("此工具需要 Windows 7 或更高版本"), _T("This tool requires Windows 7 or later") },
    // 15: 架构不支持
    { _T("此工具只适用于32或64位系统"), _T("This tool is only for 32-bit or 64-bit systems") }
};
#define MSG(id) g_msg[id][g_lang]

// 全局变量：当前可执行文件所在目录
TCHAR g_baseDir[MAX_PATH];

// 运行库名称映射（中英文独立）
typedef struct {
    TCHAR* key;
    TCHAR* nameZh;
    TCHAR* nameEn;
} PackageEntry;

PackageEntry packageMap[] = {
    { _T("v8_86"),  _T("Microsoft Visual C++ 2005 Redistributable x86"), _T("Microsoft Visual C++ 2005 Redistributable x86") },
    { _T("v8_64"),  _T("Microsoft Visual C++ 2005 Redistributable x64"), _T("Microsoft Visual C++ 2005 Redistributable x64") },
    { _T("v9_86"),  _T("Microsoft Visual C++ 2008 Redistributable x86"), _T("Microsoft Visual C++ 2008 Redistributable x86") },
    { _T("v9_64"),  _T("Microsoft Visual C++ 2008 Redistributable x64"), _T("Microsoft Visual C++ 2008 Redistributable x64") },
    { _T("v10_86"), _T("Microsoft Visual C++ 2010 Redistributable x86"), _T("Microsoft Visual C++ 2010 Redistributable x86") },
    { _T("v10_64"), _T("Microsoft Visual C++ 2010 Redistributable x64"), _T("Microsoft Visual C++ 2010 Redistributable x64") },
    { _T("v11_86"), _T("Microsoft Visual C++ 2012 Redistributable x86"), _T("Microsoft Visual C++ 2012 Redistributable x86") },
    { _T("v11_64"), _T("Microsoft Visual C++ 2012 Redistributable x64"), _T("Microsoft Visual C++ 2012 Redistributable x64") },
    { _T("v12_86"), _T("Microsoft Visual C++ 2013 Redistributable x86"), _T("Microsoft Visual C++ 2013 Redistributable x86") },
    { _T("v12_64"), _T("Microsoft Visual C++ 2013 Redistributable x64"), _T("Microsoft Visual C++ 2013 Redistributable x64") },
    { _T("v14_86"), _T("Microsoft Visual C++ v14 Redistributable x86"), _T("Microsoft Visual C++ v14 Redistributable x86") },
    { _T("v14_64"), _T("Microsoft Visual C++ v14-2022 Redistributable x64"), _T("Microsoft Visual C++ v14-2022 Redistributable x64") },
    { _T("net48"),  _T(".NET Framework 4.8"), _T(".NET Framework 4.8") }
};
#define PACKAGE_COUNT (sizeof(packageMap)/sizeof(PackageEntry))

// 获取包名称（根据当前语言）
TCHAR* GetPackageName(TCHAR* key) {
    for (int i = 0; i < PACKAGE_COUNT; i++) {
        if (_tcscmp(packageMap[i].key, key) == 0)
            return (g_lang == LANG_ZH) ? packageMap[i].nameZh : packageMap[i].nameEn;
    }
    return _T("Unknown");
}

// 清屏
void Cls() { system("cls"); }

// 等待 Enter 键
void PressEnterToContinue(int msgId) {
    _tprintf(_T("%s"), MSG(msgId));
    while (getchar() != '\n');
}

// 获取真实 Windows 版本
void GetRealWindowsVersion(int* major, int* minor) {
    HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
    if (hNtdll) {
        typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW osvi = { sizeof(osvi) };
            if (RtlGetVersion(&osvi) == 0) {
                *major = (int)osvi.dwMajorVersion;
                *minor = (int)osvi.dwMinorVersion;
                return;
            }
        }
    }
    OSVERSIONINFOEXW osvi = { sizeof(osvi) };
    GetVersionExW((OSVERSIONINFOW*)&osvi);
    *major = (int)osvi.dwMajorVersion;
    *minor = (int)osvi.dwMinorVersion;
}

// 系统检查
void SystemCheck(TCHAR* archOut, int* winVerMajor) {
    int major, minor;
    GetRealWindowsVersion(&major, &minor);
    if (major < 6) {
        _tprintf(_T("%s\n"), MSG(14));
        PressEnterToContinue(10);
        exit(1);
    }
    *winVerMajor = major;

    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        _tcscpy(archOut, _T("x64"));
    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        _tcscpy(archOut, _T("x86"));
    else {
        _tprintf(_T("%s\n"), MSG(15));
        PressEnterToContinue(10);
        exit(1);
    }
}

// 运行命令（静默）
DWORD RunCommand(LPTSTR cmdLine) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};
    TCHAR* cmdCopy = _tcsdup(cmdLine);
    if (!cmdCopy) return 1;
    BOOL success = CreateProcess(NULL, cmdCopy, NULL, NULL, FALSE,
                                  CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmdCopy);
    if (!success) {
        _tprintf(MSG(13), GetLastError());
        return 1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exitCode;
}

// 安装 VC2005/2008/2010（总是安装）
void InstallBelow2010(TCHAR* arch) {
    TCHAR path[MAX_PATH];
    _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v8\\x86\\vcredist.msi\" /qb /norestart"), g_baseDir);
    _tprintf(MSG(12), GetPackageName(_T("v8_86"))); _tprintf(_T("\n"));
    RunCommand(path);
    _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v9\\x86\\vc_red.msi\" /qb /norestart"), g_baseDir);
    _tprintf(MSG(12), GetPackageName(_T("v9_86"))); _tprintf(_T("\n"));
    RunCommand(path);
    _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v10\\x86\\vc_red.msi\" PATCH=\"%s\\resources\\vcredist\\v10\\x86\\msp_kb2565063.msp\" /qb /norestart"),
              g_baseDir, g_baseDir);
    _tprintf(MSG(12), GetPackageName(_T("v10_86"))); _tprintf(_T("\n"));
    RunCommand(path);

    if (_tcscmp(arch, _T("x64")) == 0) {
        _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v8\\x64\\vcredist.msi\" /qb /norestart"), g_baseDir);
        _tprintf(MSG(12), GetPackageName(_T("v8_64"))); _tprintf(_T("\n"));
        RunCommand(path);
        _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v9\\x64\\vc_red.msi\" /qb /norestart"), g_baseDir);
        _tprintf(MSG(12), GetPackageName(_T("v9_64"))); _tprintf(_T("\n"));
        RunCommand(path);
        _stprintf(path, _T("msiexec.exe /i \"%s\\resources\\vcredist\\v10\\x64\\vc_red.msi\" PATCH=\"%s\\resources\\vcredist\\v10\\x64\\msp_kb2565063.msp\" /qb /norestart"),
                  g_baseDir, g_baseDir);
        _tprintf(MSG(12), GetPackageName(_T("v10_64"))); _tprintf(_T("\n"));
        RunCommand(path);
    }
}

// 安装 VC2012+（仅缺失的）
void InstallAbove2012(TCHAR** missingList, int missingCount) {
    TCHAR exePath[MAX_PATH];
    for (int i = 0; i < missingCount; i++) {
        TCHAR* key = missingList[i];
        if (_tcscmp(key, _T("v11_86")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v11\\VC_redist.x86.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v11_86"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        } else if (_tcscmp(key, _T("v11_64")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v11\\VC_redist.x64.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v11_64"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        } else if (_tcscmp(key, _T("v12_86")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v12\\VC_redist.x86.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v12_86"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        } else if (_tcscmp(key, _T("v12_64")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v12\\VC_redist.x64.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v12_64"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        } else if (_tcscmp(key, _T("v14_86")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v14\\VC_redist.x86.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v14_86"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        } else if (_tcscmp(key, _T("v14_64")) == 0) {
            _stprintf(exePath, _T("\"%s\\resources\\vcredist\\v14\\VC_redist.x64.exe\" /install /passive /norestart"), g_baseDir);
            _tprintf(MSG(12), GetPackageName(_T("v14_64"))); _tprintf(_T("\n"));
            RunCommand(exePath);
        }
    }
}

// 检测缺失 VC 运行库
TCHAR** GetMissingVCRuntime(TCHAR* arch, int* outCount) {
    typedef struct { TCHAR* version; TCHAR* archFlag; TCHAR* dllName; } DllEntry;
    DllEntry dllMap[] = {
        { _T("v10"), _T("86"), _T("msvcr100.dll") }, { _T("v10"), _T("64"), _T("msvcr100.dll") },
        { _T("v11"), _T("86"), _T("msvcr110.dll") }, { _T("v11"), _T("64"), _T("msvcr110.dll") },
        { _T("v12"), _T("86"), _T("msvcr120.dll") }, { _T("v12"), _T("64"), _T("msvcr120.dll") },
        { _T("v14"), _T("86"), _T("vcruntime140.dll") }, { _T("v14"), _T("64"), _T("vcruntime140.dll") }
    };
    int mapSize = sizeof(dllMap)/sizeof(DllEntry);
    TCHAR systemRoot[MAX_PATH];
    GetEnvironmentVariable(_T("SystemRoot"), systemRoot, MAX_PATH);
    if (_tcslen(systemRoot) == 0) _tcscpy(systemRoot, _T("C:\\Windows"));
    TCHAR** missing = (TCHAR**)malloc(sizeof(TCHAR*) * 8);
    int missingIdx = 0;
    int archsToCheck[2], archCount = 0;
    if (_tcscmp(arch, _T("x86")) == 0) {
        archsToCheck[0] = 0; archCount = 1;
    } else {
        archsToCheck[0] = 1; archsToCheck[1] = 0; archCount = 2;
    }
    for (int i = 0; i < mapSize; i++) {
        int targetIs64 = (_tcscmp(dllMap[i].archFlag, _T("64")) == 0);
        bool needCheck = false;
        for (int j = 0; j < archCount; j++)
            if ((archsToCheck[j] == 1 && targetIs64) || (archsToCheck[j] == 0 && !targetIs64)) { needCheck = true; break; }
        if (!needCheck) continue;
        TCHAR dllPath[MAX_PATH];
        if (!targetIs64) {
            if (_tcscmp(arch, _T("x64")) == 0)
                _stprintf(dllPath, _T("%s\\SysWOW64\\%s"), systemRoot, dllMap[i].dllName);
            else
                _stprintf(dllPath, _T("%s\\System32\\%s"), systemRoot, dllMap[i].dllName);
        } else {
            if (_tcscmp(arch, _T("x86")) == 0) continue;
            _stprintf(dllPath, _T("%s\\System32\\%s"), systemRoot, dllMap[i].dllName);
        }
        if (GetFileAttributes(dllPath) == INVALID_FILE_ATTRIBUTES) {
            TCHAR* key = (TCHAR*)malloc(16 * sizeof(TCHAR));
            _stprintf(key, _T("%s_%s"), dllMap[i].version, dllMap[i].archFlag);
            missing[missingIdx++] = key;
        }
    }
    *outCount = missingIdx;
    return missing;
}

// 检测 .NET 4.8
bool IsDotNet48Installed() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
        return false;
    DWORD release = 0, size = sizeof(DWORD);
    RegQueryValueEx(hKey, _T("Release"), NULL, NULL, (LPBYTE)&release, &size);
    RegCloseKey(hKey);
    return release >= 528040;
}

void InstallDotNet48() {
    TCHAR cmd[MAX_PATH];
    _stprintf(cmd, _T("\"%s\\resources\\NDP48-x86-x64-AllOS-ENU.exe\" /passive /norestart /lcid=2052"), g_baseDir);
    _tprintf(MSG(12), GetPackageName(_T("net48"))); _tprintf(_T("\n"));
    RunCommand(cmd);
}

void GetBaseDir() {
    GetModuleFileName(NULL, g_baseDir, MAX_PATH);
    TCHAR* lastSlash = _tcsrchr(g_baseDir, '\\');
    if (lastSlash) *lastSlash = 0;
}

// 检测系统语言并解析命令行参数
void InitLanguage(int argc, TCHAR* argv[]) {
    // 默认检测系统 UI 语言
    LANGID langId = GetUserDefaultUILanguage();
    if (PRIMARYLANGID(langId) == LANG_CHINESE)
        g_lang = LANG_ZH;
    else
        g_lang = LANG_EN;

    // 解析参数 /l zh 或 /l en
    for (int i = 1; i < argc; i++) {
        if (_tcsicmp(argv[i], _T("/l")) == 0 && i + 1 < argc) {
            if (_tcsicmp(argv[i+1], _T("zh")) == 0) g_lang = LANG_ZH;
            else if (_tcsicmp(argv[i+1], _T("en")) == 0) g_lang = LANG_EN;
            break;
        }
    }
}

int _tmain(int argc, TCHAR* argv[]) {
    // 设置控制台 UTF-8 输出
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    InitLanguage(argc, argv);

    _tprintf(_T("%s\n"), MSG(0));
    _tprintf(_T("%s\n"), MSG(1));
    _tprintf(_T("%s\n"), MSG(2));

    GetBaseDir();
    TCHAR arch[4];
    int winVer;
    SystemCheck(arch, &winVer);

    int vcMissingCount;
    TCHAR** vcMissing = GetMissingVCRuntime(arch, &vcMissingCount);
    bool dotnetMissing = !IsDotNet48Installed();

    _tprintf(_T("%s%s %d %s\n"), MSG(3), _T("Windows"), winVer, arch);
    _tprintf(_T("\n"));
    _tprintf(_T("%s\n"), MSG(4));

    if (vcMissingCount == 0 && !dotnetMissing) {
        _tprintf(_T("%s\n"), MSG(5));
        PressEnterToContinue(10);
        return 0;
    }

    _tprintf(_T("%s\n"), MSG(6));
    for (int i = 0; i < vcMissingCount; i++)
        _tprintf(_T("  %s\n"), GetPackageName(vcMissing[i]));
    if (dotnetMissing)
        _tprintf(_T("  %s\n"), GetPackageName(_T("net48")));
    _tprintf(MSG(7));
    _tprintf(_T("\n"));
    PressEnterToContinue(8);

    Cls();
    InstallBelow2010(arch);
    InstallAbove2012(vcMissing, vcMissingCount);
    if (dotnetMissing) InstallDotNet48();

    _tprintf(MSG(9));
    _tprintf(_T("\n"));
    _tprintf(_T("  %s\n"), GetPackageName(_T("v8_86")));
    _tprintf(_T("  %s\n"), GetPackageName(_T("v8_64")));
    _tprintf(_T("  %s\n"), GetPackageName(_T("v9_86")));
    _tprintf(_T("  %s\n"), GetPackageName(_T("v9_64")));
    if (dotnetMissing) _tprintf(_T("  %s\n"), GetPackageName(_T("net48")));
    for (int i = 0; i < vcMissingCount; i++)
        _tprintf(_T("  %s\n"), GetPackageName(vcMissing[i]));

    for (int i = 0; i < vcMissingCount; i++) free(vcMissing[i]);
    free(vcMissing);

    PressEnterToContinue(10);
    return 0;
}