/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifdef _WIN32

#include "fs/fs.hpp"
#include <Windows.h>
#include <UserEnv.h>

#pragma comment(lib, "userenv.lib")

// -----------------------------------------------------------------------------
// path
std::string fs::root()
{
    wchar_t buf[MAX_PATH]{};
    return ::GetSystemWindowsDirectoryW(buf, _countof(buf)) >= 3 ? fs::narrow(buf).substr(0, 3) : "";
}

std::string fs::home()
{
    HANDLE token = 0;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &token))
        return "";

    wchar_t buf[MAX_PATH]{};
    DWORD   len = _countof(buf);
    BOOL    ret = ::GetUserProfileDirectoryW(token, buf, &len);

    ::CloseHandle(token);

    return ret ? fs::prune(fs::narrow(buf)) : "";
}

std::string fs::tmp()
{
    wchar_t buf[MAX_PATH]{};
    return ::GetTempPathW(_countof(buf), buf) ? fs::prune(fs::narrow(buf)) : "";
}

std::string fs::cwd()
{
    wchar_t buf[MAX_PATH]{};
    return ::GetCurrentDirectoryW(_countof(buf), buf) ? fs::prune(fs::narrow(buf)) : "";
}

char fs::sep()
{
    return '\\';
}

std::vector<std::string> fs::drives()
{
    std::vector<std::string> ret;

    DWORD bits = ::GetLogicalDrives();

    for (auto i = 0; i < 26; ++i)  // A to Z
    {
        if (bits & (1 << i))
            ret.emplace_back(1, 'A' + i);
    }

    return ret;
}

#endif