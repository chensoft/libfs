/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include <fstream>
#include <cstring>
#include <climits>
#include <cstdio>
#include <queue>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <pwd.h>

// -----------------------------------------------------------------------------
// helper
namespace fs
{
    class dir_handle final
    {
    public:
        dir_handle(DIR *d = nullptr) : val(d) {}
        ~dir_handle() { val ? ::closedir(val) : 0; }

        DIR *val;
    };
}

// -----------------------------------------------------------------------------
// path
std::string fs::root()
{
    return "/";
}

std::string fs::user()
{
    auto pwd = ::getpwuid(::getuid());
    return pwd ? pwd->pw_name : "";
}

std::string fs::home()
{
    // check env first
    auto env = ::getenv("HOME");
    if (env)
        return fs::prune(env);

    // check passwd file
    auto pwd = ::getpwuid(::getuid());
    return pwd ? fs::prune(pwd->pw_dir) : "";
}

std::string fs::home(const std::string &user)
{
    auto pwd = ::getpwnam(user.c_str());
    return pwd ? fs::prune(pwd->pw_dir) : "";
}

std::string fs::tmp()
{
    auto env = ::getenv("TMPDIR");
    return env ? fs::prune(env) : "/tmp";
}

std::string fs::cwd()
{
    char buf[PATH_MAX]{};
    return ::getcwd(buf, sizeof(buf)) ? fs::prune(buf) : "";
}

char fs::sep()
{
    return '/';
}

std::vector<std::string> fs::drives()
{
    return {"/"};
}

// -----------------------------------------------------------------------------
// split
std::string fs::realpath(std::string path)
{
    path = fs::normalize(std::move(path));

    if (fs::isRelative(path))
        path.replace(0, 0, fs::cwd() + fs::sep());

    char buf[PATH_MAX]{};
    return ::realpath(path.c_str(), buf) ? buf : path;
}

// -----------------------------------------------------------------------------
// check
bool fs::isExist(const std::string &path, bool follow_symlink)
{
    if (follow_symlink)
        return !::access(path.c_str(), F_OK);

    struct ::stat st{};
    return !::lstat(path.c_str(), &st);
}

bool fs::isEmpty(const std::string &path)
{
    // treat not exist as empty
    struct ::stat st{};
    if (::stat(path.c_str(), &st))
        return true;

    // check file contents
    if (S_ISREG(st.st_mode))
        return !fs::filesize(path);

    // check dir has entries
    bool empty = true;

    fs::walk(path, [&](WalkEntry *entry) {
        empty = false;
        entry->stop = true;
    }, false);

    return empty;
}

bool fs::isDir(const std::string &path, bool follow_symlink)
{
    struct ::stat st{};
    return (follow_symlink ? !::stat(path.c_str(), &st) : !::lstat(path.c_str(), &st)) && S_ISDIR(st.st_mode);
}

bool fs::isFile(const std::string &path, bool follow_symlink)
{
    struct ::stat st{};
    return (follow_symlink ? !::stat(path.c_str(), &st) : !::lstat(path.c_str(), &st)) && S_ISREG(st.st_mode);
}

bool fs::isSymlink(const std::string &path)
{
    struct ::stat st{};
    return !::lstat(path.c_str(), &st) && S_ISLNK(st.st_mode);
}

// -----------------------------------------------------------------------------
// mode
bool fs::isReadable(const std::string &path)
{
    return !::access(path.c_str(), R_OK);
}

bool fs::isWritable(const std::string &path)
{
    return !::access(path.c_str(), W_OK);
}

bool fs::isExecutable(const std::string &path)
{
    return !::access(path.c_str(), X_OK);
}

// -----------------------------------------------------------------------------
// property
fs::status fs::filetime(const std::string &path, struct ::timespec *access, struct ::timespec *modify, struct ::timespec *create)
{
    struct ::stat st{};
    if (::stat(path.c_str(), &st))
        return status(errno);

#ifdef __linux__
    if (access)
        *access = st.st_atim;

    if (modify)
        *modify = st.st_mtim;

    if (create)
        *create = st.st_ctim;
#else
    if (access)
        *access = st.st_atimespec;

    if (modify)
        *modify = st.st_mtimespec;

    if (create)
        *create = st.st_ctimespec;
#endif

    return {};
}

struct ::timespec fs::atime(const std::string &path)
{
    struct ::timespec time{};
    fs::filetime(path, &time, nullptr, nullptr);
    return time;
}

struct ::timespec fs::mtime(const std::string &path)
{
    struct ::timespec time{};
    fs::filetime(path, nullptr, &time, nullptr);
    return time;
}

struct ::timespec fs::ctime(const std::string &path)
{
    struct ::timespec time{};
    fs::filetime(path, nullptr, nullptr, &time);
    return time;
}

std::size_t fs::filesize(const std::string &file)
{
    struct ::stat st{};
    return !::stat(file.c_str(), &st) ? static_cast<std::size_t>(st.st_size) : 0;
}

// -----------------------------------------------------------------------------
// operation
fs::status fs::chdir(const std::string &dir_new, std::string *dir_old)
{
    if (dir_old)
        *dir_old = fs::cwd();

    return !::chdir(dir_new.c_str()) ? status() : status(errno);
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
    struct ::utimbuf time{atime, mtime};
    return !::utime(file.c_str(), &time) ? status() : status(errno);
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

    return dir.empty() || !::mkdir(dir.c_str(), mode) || errno == EEXIST || errno == EISDIR ? status() : status(errno);
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

    return !::rename(path_old.c_str(), path_new.c_str()) ? status() : status(errno);
}

fs::status fs::remove(const std::string &path)
{
    if (!::remove(path.c_str()) || errno == ENOENT)
        return {};

    if (errno != ENOTEMPTY)
        return status(errno);

    auto error = 0;

    fs::walk(path, [&](WalkEntry *entry) {
        if (::remove(entry->path().c_str()))
        {
            entry->stop = true;
            error = errno;
        }
    }, true, WalkStrategy::DeepestFirst);

    return !error && ::remove(path.c_str()) ? status(errno) : status(error);
}

// -----------------------------------------------------------------------------
// visit
static void visit_children_first(const std::string &directory, const std::function<void (fs::WalkEntry *entry)> &callback, bool recursive)
{
    fs::dir_handle ptr = ::opendir(directory.c_str());
    if (!ptr.val)
        return;

    dirent *item{};

    while ((item = ::readdir(ptr.val)))
    {
        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
            continue;

        fs::WalkEntry entry;
        entry.root = directory;
        entry.name = item->d_name;

        callback(&entry);
        if (entry.stop)
            return;

        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))  // some filesystem will return DT_UNKNOWN
            fs::walk(entry.path(), callback, recursive);
    }
}

static bool visit_siblings_first(const std::string &directory, const std::function<void (fs::WalkEntry *entry)> &callback, bool recursive)
{
    fs::dir_handle ptr = ::opendir(directory.c_str());
    if (!ptr.val)
        return false;

    dirent *item{};
    std::queue<std::string> queue;

    while ((item = ::readdir(ptr.val)))
    {
        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
            continue;

        fs::WalkEntry entry;
        entry.root = directory;
        entry.name = item->d_name;

        callback(&entry);
        if (entry.stop)
            return true;

        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))
            queue.emplace(entry.path());
    }

    while (!queue.empty())
    {
        auto folder = std::move(queue.front());
        queue.pop();

        if (visit_siblings_first(folder, callback, recursive))
            return true;
    }

    return false;
}

static void visit_deepest_first(const std::string &directory, const std::function<void (fs::WalkEntry *entry)> &callback, bool recursive)
{
    fs::dir_handle ptr = ::opendir(directory.c_str());
    if (!ptr.val)
        return;

    dirent *item{};

    while ((item = ::readdir(ptr.val)))
    {
        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
            continue;

        fs::WalkEntry entry;
        entry.root = directory;
        entry.name = item->d_name;

        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))
            fs::walk(entry.path(), callback, recursive);

        callback(&entry);
        if (entry.stop)
            return;
    }
}

void fs::walk(const std::string &directory, const std::function<void (WalkEntry *entry)> &callback, bool recursive, WalkStrategy strategy)
{
    switch (strategy)
    {
        case WalkStrategy::ChildrenFirst:
            visit_children_first(directory, callback, recursive);
            break;

        case WalkStrategy::SiblingsFirst:
            visit_siblings_first(directory, callback, recursive);
            break;

        case WalkStrategy::DeepestFirst:
            visit_deepest_first(directory, callback, recursive);
            break;
    }
}

#endif