/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#ifdef _WIN32

#include "fs/fs.hpp"
#include <fstream>
#include <queue>
#include <sys/utime.h>
#include <Windows.h>
#include <UserEnv.h>
#include <Lmcons.h>

#pragma comment(lib, "userenv.lib")

// -----------------------------------------------------------------------------
// helper
namespace fs
{
    class file_handle final
    {
    public:
        file_handle(HANDLE h = INVALID_HANDLE_VALUE) : val(h) {}
        ~file_handle() { ::CloseHandle(val); }

        HANDLE val;
    };

    class find_handle final
    {
    public:
        find_handle(HANDLE h = INVALID_HANDLE_VALUE) : val(h) {}
        ~find_handle() { ::FindClose(val); }

        HANDLE val;
    };
}

// -----------------------------------------------------------------------------
// path
std::string fs::root()
{
    wchar_t buf[MAX_PATH]{};
    return ::GetSystemWindowsDirectoryW(buf, _countof(buf)) >= 3 ? fs::narrow(buf).substr(0, 3) : "";
}

std::string fs::user()
{
    wchar_t buf[UNLEN + 1]{};
    DWORD   len = _countof(buf);
    return ::GetUserNameW(buf, &len) ? fs::narrow(buf) : "";
}

std::string fs::home()
{
    fs::file_handle token;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &token.val))
        return "";

    wchar_t buf[MAX_PATH]{};
    DWORD   len = _countof(buf);
    return ::GetUserProfileDirectoryW(token.val, buf, &len) ? fs::prune(fs::narrow(buf)) : "";
}

std::string fs::home(const std::string &user)
{
    auto cur = fs::dirname(fs::home());
    auto ret = cur + fs::sep() + user;
    return fs::isDir(ret) ? ret : "";
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
    std::string suffix(":\\");

    DWORD bits = ::GetLogicalDrives();

    for (auto i = 0; i < 26; ++i)  // A to Z
    {
        if (bits & (1 << i))
            ret.emplace_back(std::string(1, 'A' + i) + suffix);
    }

    return ret;
}

// -----------------------------------------------------------------------------
// split
std::string fs::realpath(std::string path)
{
    path = fs::normalize(std::move(path));

    if (fs::isRelative(path))
        path.replace(0, 0, fs::cwd() + fs::sep());

    fs::file_handle handle = ::CreateFileW(fs::widen(path).c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (handle.val == INVALID_HANDLE_VALUE)
        return path;

    wchar_t buf[MAX_PATH]{};
    if (::GetFinalPathNameByHandleW(handle.val, buf, _countof(buf), 0))
        path = fs::narrow(buf);

    return path.size() >= 4 && path.substr(0, 4) == "\\\\?\\" ? path.substr(4) : path;  // remove "\\?\" prefix
}

// -----------------------------------------------------------------------------
// check
bool fs::isExist(const std::string &path, bool follow_symlink)
{
    return ::GetFileAttributesW(fs::widen(follow_symlink ? fs::realpath(path) : path).c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool fs::isEmpty(const std::string &path)
{
    // treat not exist as empty
    if (!fs::isExist(path))
        return true;

    // check file contents
    if (fs::isFile(path))
        return !fs::filesize(path);

    // check dir has entries
    bool empty = true;

    fs::visit(path, [&](const std::string &, bool *stop) {
        empty = false;
        *stop = true;
    }, false);

    return empty;
}

bool fs::isDir(const std::string &path, bool follow_symlink)
{
    DWORD attr = ::GetFileAttributesW(fs::widen(follow_symlink ? fs::realpath(path) : path).c_str());
    return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_DIRECTORY;
}

bool fs::isFile(const std::string &path, bool follow_symlink)
{
    DWORD attr = ::GetFileAttributesW(fs::widen(follow_symlink ? fs::realpath(path) : path).c_str());
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool fs::isSymlink(const std::string &path)
{
    DWORD attr = ::GetFileAttributesW(fs::widen(path).c_str());
    return attr != INVALID_FILE_ATTRIBUTES && attr & FILE_ATTRIBUTE_REPARSE_POINT;
}

// -----------------------------------------------------------------------------
// mode
bool fs::isReadable(const std::string &path)
{
    // see https://msdn.microsoft.com/en-us/library/1w06ktdy.aspx
    // 0: Existence only, 2: Write-only, 4: Read-only, 6: Read and write
    return !::_waccess(fs::widen(path).c_str(), 6) || !::_waccess(fs::widen(path).c_str(), 4);
}

bool fs::isWritable(const std::string &path)
{
    return !::_waccess(fs::widen(path).c_str(), 6) || !::_waccess(fs::widen(path).c_str(), 2);
}

bool fs::isExecutable(const std::string &path)
{
    DWORD type = 0;
    return fs::isDir(path) || ::GetBinaryTypeW(fs::widen(path).c_str(), &type);
}

// -----------------------------------------------------------------------------
// property
fs::status fs::filetime(const std::string &path, struct ::timespec *access, struct ::timespec *modify, struct ::timespec *create)
{
    fs::file_handle handle = ::CreateFileW(fs::widen(path).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle.val == INVALID_HANDLE_VALUE)
        return status(::GetLastError());

    FILETIME access_time{}, modify_time{}, create_time{};
    if (!::GetFileTime(handle.val, &create_time, &access_time, &modify_time))  // create time is the first
        return status(::GetLastError());

    ULARGE_INTEGER large_time{};

    auto ticks = 10000000ull;     // FILETIME ticks are in 100 nanoseconds
    auto epoch = 11644473600ull;  // FILETIME epoch is 1601-01-01T00:00:00Z

    if (access)
    {
        large_time.LowPart  = access_time.dwLowDateTime;
        large_time.HighPart = access_time.dwHighDateTime;

        access->tv_sec  = static_cast<decltype(access->tv_sec)>(large_time.QuadPart / ticks - epoch);
        access->tv_nsec = static_cast<decltype(access->tv_nsec)>(large_time.QuadPart - large_time.QuadPart / ticks * ticks);
    }

    if (modify)
    {
        large_time.LowPart = modify_time.dwLowDateTime;
        large_time.HighPart = modify_time.dwHighDateTime;

        modify->tv_sec  = static_cast<decltype(modify->tv_sec)>(large_time.QuadPart / ticks - epoch);
        modify->tv_nsec = static_cast<decltype(modify->tv_nsec)>(large_time.QuadPart - large_time.QuadPart / ticks * ticks);
    }

    if (create)
    {
        large_time.LowPart = create_time.dwLowDateTime;
        large_time.HighPart = create_time.dwHighDateTime;

        create->tv_sec  = static_cast<decltype(create->tv_sec)>(large_time.QuadPart / ticks - epoch);
        create->tv_nsec = static_cast<decltype(create->tv_nsec)>(large_time.QuadPart - large_time.QuadPart / ticks * ticks);
    }

    return {};
}

struct ::timespec fs::atime(const std::string &path)
{
    struct ::timespec time {};
    fs::filetime(path, &time, nullptr, nullptr);
    return time;
}

struct ::timespec fs::mtime(const std::string &path)
{
    struct ::timespec time {};
    fs::filetime(path, nullptr, &time, nullptr);
    return time;
}

struct ::timespec fs::ctime(const std::string &path)
{
    struct ::timespec time {};
    fs::filetime(path, nullptr, nullptr, &time);
    return time;
}

std::size_t fs::filesize(const std::string &file)
{
    fs::file_handle handle = ::CreateFileW(fs::widen(file).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return handle.val != INVALID_HANDLE_VALUE ? ::GetFileSize(handle.val, NULL) : 0;
}

// -----------------------------------------------------------------------------
// operation
fs::status fs::chdir(const std::string &dir_new, std::string *dir_old)
{
    if (dir_old)
        *dir_old = fs::cwd();

    return ::SetCurrentDirectoryW(fs::widen(dir_new).c_str()) ? status() : status(::GetLastError());
}

fs::status fs::touch(const std::string &file, std::time_t atime, std::time_t mtime)
{
    // using current time if it's zero
    if (!atime)
        atime = ::time(nullptr);

    if (!mtime)
        mtime = ::time(nullptr);

    // create parent directory
    auto result = fs::mkdir(fs::dirname(file));
    if (!result)
        return result;

    // create file if not exist
    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
    if (!out)
        return status(errno);

    // modify mtime and atime
    ::_utimbuf time{atime, mtime};
    return !::_wutime(fs::widen(file).c_str(), &time) ? status() : status(errno);
}

fs::status fs::rename(const std::string &path_old, const std::string &path_new)
{
    // remove existence path
    auto result = fs::remove(path_new);
    if (!result)
        return result;

    // create new directory
    result = fs::mkdir(fs::dirname(path_new));
    if (!result)
        return result;

    return !::_wrename(fs::widen(path_old).c_str(), fs::widen(path_new).c_str()) ? status() : status(errno);
}

fs::status fs::remove(const std::string &path)
{
    if (::DeleteFileW(fs::widen(path).c_str()) || ::RemoveDirectoryW(fs::widen(path).c_str()) || ::GetLastError() == ERROR_FILE_NOT_FOUND)
        return {};

    auto error = 0;

    fs::visit(path, [&](const std::string &item, bool *stop) {
        if (!::DeleteFileW(fs::widen(item).c_str()) && !::RemoveDirectoryW(fs::widen(item).c_str()))
        {
            *stop = true;
            error = ::GetLastError();
        }
    }, true, VisitStrategy::DeepestFirst);

    return error ? status(error) : (::DeleteFileW(fs::widen(path).c_str()) || ::RemoveDirectoryW(fs::widen(path).c_str()) ? status() : status(::GetLastError()));
}

fs::status fs::mkdir(const std::string &dir, std::uint16_t mode)
{
    auto parent = fs::dirname(dir);

    if (!parent.empty() && !fs::isDir(parent))
    {
        auto result = fs::mkdir(parent, mode);
        if (!result)
            return result;
    }

    return dir.empty() || fs::isDir(dir) || ::CreateDirectoryW(fs::widen(dir).c_str(), NULL) || ::GetLastError() == ERROR_ALREADY_EXISTS ? status() : status(::GetLastError());
}

// -----------------------------------------------------------------------------
// traversal
static void visit_children_first(const std::string &dir, const std::function<void(const std::string &path, bool *stop)> &callback, bool recursive)
{
    WIN32_FIND_DATAW item{};
    fs::find_handle ptr = ::FindFirstFileW(fs::widen(dir + "\\*").c_str(), &item);

    if (ptr.val == INVALID_HANDLE_VALUE)
        return;

    bool stop = false;

    do
    {
        std::string name(fs::narrow(item.cFileName));
        if (name == "." || name == "..")
            continue;

        std::string path(dir + fs::sep());
        path += name;

        callback(path, &stop);
        if (stop)
            return;

        if (recursive && item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            fs::visit(path, callback, recursive);
    } while (::FindNextFileW(ptr.val, &item));
}

static bool visit_siblings_first(const std::string &dir, const std::function<void(const std::string &path, bool *stop)> &callback, bool recursive)
{
    WIN32_FIND_DATAW item{};
    fs::find_handle ptr = ::FindFirstFileW(fs::widen(dir + "\\*").c_str(), &item);

    if (ptr.val == INVALID_HANDLE_VALUE)
        return false;

    bool stop = false;

    std::queue<std::string> queue;

    do
    {
        std::string name(fs::narrow(item.cFileName));
        if (name == "." || name == "..")
            continue;

        std::string path(dir + fs::sep());
        path += name;

        callback(path, &stop);
        if (stop)
            return true;

        if (recursive && item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            queue.emplace(std::move(path));
    } while (::FindNextFileW(ptr.val, &item));

    while (!queue.empty())
    {
        auto folder = std::move(queue.front());
        queue.pop();

        if (visit_siblings_first(folder, callback, recursive))
            return true;
    }

    return false;
}

static void visit_deepest_first(const std::string &dir, const std::function<void(const std::string &path, bool *stop)> &callback, bool recursive)
{
    WIN32_FIND_DATAW item{};
    fs::find_handle ptr = ::FindFirstFileW(fs::widen(dir + "\\*").c_str(), &item);

    if (ptr.val == INVALID_HANDLE_VALUE)
        return;

    bool stop = false;

    do
    {
        std::string name(fs::narrow(item.cFileName));
        if (name == "." || name == "..")
            continue;

        std::string path(dir + fs::sep());
        path += name;

        if (recursive && item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            fs::visit(path, callback, recursive);

        callback(path, &stop);
        if (stop)
            return;
    } while (::FindNextFileW(ptr.val, &item));
}

void fs::visit(const std::string &dir, const std::function<void(const std::string &path, bool *stop)> &callback, bool recursive, VisitStrategy strategy)
{
    switch (strategy)
    {
    case VisitStrategy::ChildrenFirst:
        visit_children_first(dir, callback, recursive);
        break;

    case VisitStrategy::SiblingsFirst:
        visit_siblings_first(dir, callback, recursive);
        break;

    case VisitStrategy::DeepestFirst:
        visit_deepest_first(dir, callback, recursive);
        break;
    }
}

#endif