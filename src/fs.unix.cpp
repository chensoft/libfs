/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include <cstring>
#include <climits>
#include <cstdio>
#include <memory>
#include <queue>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <pwd.h>

// todo PATH_MAX is not enough, check path greater than PATH_MAX
// todo handle UNC pathnames

// -----------------------------------------------------------------------------
// path
std::string fs::root()
{
    return "/";
}

std::string fs::home()
{
    // check env first
    auto env = ::getenv("HOME");
    if (env)
        return fs::prune(env);

    // check passwd file
    struct ::passwd  pwd = {};
    struct ::passwd *ret = nullptr;

    char buf[std::max(1024l, ::sysconf(_SC_GETPW_R_SIZE_MAX))];
    ::getpwuid_r(::getuid(), &pwd, buf, sizeof(buf), &ret);

    return ret ? fs::prune(pwd.pw_dir) : "";
}

std::string fs::tmp()
{
    auto env = ::getenv("TMPDIR");
    return env ? fs::prune(env) : "/tmp";
}

std::string fs::cwd()
{
    char buf[PATH_MAX];
    return ::getcwd(buf, sizeof(buf)) ? fs::prune(buf) : "";
}

char fs::sep()
{
    return '/';
}

std::vector<std::string> fs::drives()
{
    // todo return all mounted drives
    return {"/"};
}

//// -----------------------------------------------------------------------------
//// split
//std::string fs::realpath(const std::string &path)
//{
//    // todo optimize add function visit path's segments
//    char buf[PATH_MAX];  // todo use std::string(xxx, '\0')
//
//    if (::realpath(path.c_str(), buf))
//        return buf;
//
//    return fs::isRelative(path) ? fs::cwd() + fs::sep() + fs::normalize(path) : fs::normalize(path);
//}
//
//// -----------------------------------------------------------------------------
//// check
//bool fs::isExist(const std::string &path, bool follow_symlink)
//{
//    if (follow_symlink)
//        return !::access(path.c_str(), F_OK);
//
//    struct ::stat st{};
//    return !::lstat(path.c_str(), &st);
//}
//
//bool fs::isEmpty(const std::string &path)
//{
//    // treat not exist as empty
//    struct ::stat st{};
//    if (::stat(path.c_str(), &st))
//        return true;
//
//    // check file contents
//    if (S_ISREG(st.st_mode))
//        return !fs::filesize(path);
//
//    // check dir has entries
//    bool empty = true;
//
//    fs::visit(path, [&] (const std::string &, bool *stop) {
//        empty = false;
//        *stop = true;
//    }, false);
//
//    return empty;
//}
//
//bool fs::isDir(const std::string &path, bool follow_symlink)
//{
//    struct ::stat st{};
//    return (follow_symlink ? !::stat(path.c_str(), &st) : !::lstat(path.c_str(), &st)) && S_ISDIR(st.st_mode);
//}
//
//bool fs::isFile(const std::string &path, bool follow_symlink)
//{
//    struct ::stat st{};
//    return (follow_symlink ? !::stat(path.c_str(), &st) : !::lstat(path.c_str(), &st)) && S_ISREG(st.st_mode);
//}
//
//bool fs::isSymlink(const std::string &path)
//{
//    struct ::stat st{};
//    return !::lstat(path.c_str(), &st) && S_ISLNK(st.st_mode);
//}
//
//// -----------------------------------------------------------------------------
//// mode
//bool fs::isReadable(const std::string &path)
//{
//    return !::access(path.c_str(), R_OK);
//}
//
//bool fs::isWritable(const std::string &path)
//{
//    return !::access(path.c_str(), W_OK);
//}
//
//bool fs::isExecutable(const std::string &path)
//{
//    return !::access(path.c_str(), X_OK);
//}
//
//// -----------------------------------------------------------------------------
//// property
//std::size_t fs::totalSpace(const std::string &)
//{
//    // todo
//    return 0;
//}
//
//std::size_t fs::freeSpace(const std::string &)
//{
//    // todo
//    return 0;
//}
//
//// -----------------------------------------------------------------------------
//// property
//struct ::timespec fs::atime(const std::string &path)
//{
//    struct ::stat st{};
//    if (::stat(path.c_str(), &st))
//        return {};
//
//    // todo use stat64
//#ifdef __linux__
//    return st.st_atim;
//#else
//    return st.st_atimespec;
//#endif
//}
//
//struct ::timespec fs::mtime(const std::string &path)
//{
//    struct ::stat st{};
//    if (::stat(path.c_str(), &st))
//        return {};
//
//#ifdef __linux__
//    return st.st_mtim;
//#else
//    return st.st_mtimespec;
//#endif
//}
//
//struct ::timespec fs::ctime(const std::string &path)
//{
//    struct ::stat st{};
//    if (::stat(path.c_str(), &st))
//        return {};
//
//#ifdef __linux__
//    return st.st_ctim;
//#else
//    return st.st_ctimespec;
//#endif
//}
//
//std::size_t fs::filesize(const std::string &file)
//{
//    struct ::stat st{};
//    return !::stat(file.c_str(), &st) ? static_cast<std::size_t>(st.st_size) : 0;
//}
//
//// -----------------------------------------------------------------------------
//// operation
//fs::status fs::chdir(const std::string &dir_new, std::string *dir_old)
//{
//    if (dir_old)
//        *dir_old = fs::cwd();
//
//    return !::chdir(dir_new.c_str()) ? status() : status(errno);
//}
//
//fs::status fs::touch(const std::string &file, std::time_t atime, std::time_t mtime)
//{
//    // using current time if it's zero
//    if (!atime)
//        atime = ::time(nullptr);
//
//    if (!mtime)
//        mtime = ::time(nullptr);
//
//    // create parent directory
//    auto result = fs::mkdir(fs::dirname(file));
//    if (!result)
//        return result;
//
//    // create file if not exist
//    auto deleter = [] (FILE *ptr) { ::fclose(ptr); };
//    std::unique_ptr<FILE, decltype(deleter)> handle(::fopen(file.c_str(), "ab+"), deleter);
//
//    if (!handle)
//        return status(errno);
//
//    // modify mtime and atime
//    struct ::utimbuf time{atime, mtime};
//    return !::utime(file.c_str(), &time) ? status() : status(errno);
//}
//
//fs::status fs::mkdir(const std::string &dir, std::uint16_t mode)
//{
//    auto parent = fs::dirname(dir);
//
//    if (!parent.empty() && !fs::isDir(parent, false))  // todo check other call fs::isXXX more strict, consider symbolic link
//    {
//        auto result = fs::mkdir(parent, mode);
//        if (!result)
//            return result;  // todo check others do not return unnecessary status(errno)
//    }
//
//    return dir.empty() || !::mkdir(dir.c_str(), mode) || (errno == EEXIST) ? status() : status(errno);
//}
//
//fs::status fs::remove(const std::string &path)
//{
//    if (!::remove(path.c_str()) || (errno == ENOENT))
//        return status();
//
//    if (errno != ENOTEMPTY)
//        return status(errno);
//
//    auto error = 0;
//
//    fs::visit(path, [&] (const std::string &item, bool *stop) {
//        if (::remove(item.c_str()))
//        {
//            *stop = true;
//            error = errno;
//        }
//    }, true, VisitStrategy::DeepestFirst);
//
//    return !error && ::remove(path.c_str()) ? status(errno) : status(error);
//}
//
//fs::status fs::symlink(const std::string &path, const std::string &link)
//{
//    auto result = fs::mkdir(fs::dirname(link));
//    if (!result)
//        return result;
//
//    return !::symlink(path.c_str(), link.c_str()) ? status() : status(errno);
//}
//
//// -----------------------------------------------------------------------------
//// traversal
//static void visit_children_first(const std::string &dir, const std::function<void (const std::string &path, bool *stop)> &callback, bool recursive)
//{
//    auto deleter = [] (DIR *ptr) { ::closedir(ptr); };
//    std::unique_ptr<DIR, decltype(deleter)> ptr(::opendir(dir.c_str()), deleter);
//
//    if (!ptr)
//        return;
//
//    dirent *item{};
//    bool    stop{};
//
//    // todo do not recursive?
//    while ((item = ::readdir(ptr.get())))
//    {
//        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
//            continue;
//
//        std::string path(dir + fs::sep());
//        path += item->d_name;
//
//        callback(path, &stop);
//        if (stop)
//            return;
//
//        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))
//            fs::visit(path, callback, recursive);
//    }
//}
//
//static bool visit_siblings_first(const std::string &dir, const std::function<void (const std::string &path, bool *stop)> &callback, bool recursive)
//{
//    dirent *item{};
//    bool    stop{};
//
//    auto deleter = [] (DIR *ptr) { ::closedir(ptr); };
//    std::unique_ptr<DIR, decltype(deleter)> ptr(::opendir(dir.c_str()), deleter);
//
//    if (!ptr)
//        return false;
//
//    std::queue<std::string> queue;
//
//    while ((item = ::readdir(ptr.get())))
//    {
//        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
//            continue;
//
//        std::string path(dir + fs::sep());
//        path += item->d_name;
//
//        callback(path, &stop);
//        if (stop)
//            return true;
//
//        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))
//            queue.emplace(std::move(path));
//    }
//
//    while (!queue.empty())
//    {
//        auto folder = std::move(queue.front());
//        queue.pop();
//
//        if (visit_siblings_first(folder, callback, recursive))
//            return true;
//    }
//
//    return false;
//}
//
//static void visit_deepest_first(const std::string &dir, const std::function<void (const std::string &path, bool *stop)> &callback, bool recursive)
//{
//    auto deleter = [] (DIR *ptr) { ::closedir(ptr); };
//    std::unique_ptr<DIR, decltype(deleter)> ptr(::opendir(dir.c_str()), deleter);
//
//    if (!ptr)
//        return;
//
//    dirent *item{};
//    bool    stop{};
//
//    while ((item = ::readdir(ptr.get())))
//    {
//        if ((item->d_name[0] == '.' && !item->d_name[1]) || (item->d_name[0] == '.' && item->d_name[1] == '.' && !item->d_name[2]))
//            continue;
//
//        std::string path(dir + fs::sep());
//        path += item->d_name;
//
//        if (recursive && (item->d_type == DT_DIR || item->d_type == DT_UNKNOWN))
//            fs::visit(path, callback, recursive);
//
//        callback(path, &stop);
//        if (stop)
//            return;
//    }
//}
//
//void fs::visit(const std::string &dir, const std::function<void (const std::string &path, bool *stop)> &callback, bool recursive, VisitStrategy strategy)
//{
//    switch (strategy)
//    {
//        case VisitStrategy::ChildrenFirst:
//            visit_children_first(dir, callback, recursive);
//            break;
//
//        case VisitStrategy::SiblingsFirst:
//            visit_siblings_first(dir, callback, recursive);
//            break;
//
//        case VisitStrategy::DeepestFirst:
//            visit_deepest_first(dir, callback, recursive);
//            break;
//    }
//}
//
//std::vector<std::string> fs::collect(const std::string &dir, bool recursive, VisitStrategy strategy)
//{
//    std::vector<std::string> ret;
//
//    fs::visit(dir, [&ret] (const std::string &path) {
//        ret.emplace_back(path);
//    }, recursive, strategy);
//
//    return ret;
//}

#endif