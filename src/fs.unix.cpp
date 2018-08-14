/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <utime.h>
#include <pwd.h>

// todo PATH_MAX is not enough, check path greater than PATH_MAX
// todo handle UNC pathnames

// -----------------------------------------------------------------------------
// sys
std::string fs::root()
{
    return "/";
}

std::string fs::home()
{
    // check env first
    auto env = ::getenv("HOME");
    if (env)
        return env;

    // check passwd file
    struct ::passwd  pwd = {};
    struct ::passwd *ret = nullptr;

    char buf[std::max(1024l, ::sysconf(_SC_GETPW_R_SIZE_MAX))];
    ::getpwuid_r(::getuid(), &pwd, buf, sizeof(buf), &ret);

    return ret ? pwd.pw_dir : "";
}

std::string fs::tmp()
{
    auto env = ::getenv("TMPDIR");
    return env ? env : "/tmp";
}

std::string fs::cwd()
{
    char buf[PATH_MAX];
    auto ret = ::getcwd(buf, sizeof(buf));
    return ret ? ret : "";
}

std::string fs::sep()
{
    return "/";
}

std::vector<std::string> fs::drives()
{
    // todo return all mounted drives
    return {"/"};
}

// -----------------------------------------------------------------------------
// path
std::string fs::realpath(const std::string &path)
{
    char buf[PATH_MAX];
    auto ret = ::realpath(fs::normalize(path).c_str(), buf);
    return ret ? ret : "";
}

// -----------------------------------------------------------------------------
// exist
bool fs::isExist(const std::string &path)
{
    // todo test symlink
    return !::access(path.c_str(), F_OK);
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

bool fs::isLink(const std::string &path)
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
// operate
fs::status fs::touch(const std::string &file, std::time_t mtime, std::time_t atime)
{
    // using current time if it's zero
    if (!mtime)
        mtime = ::time(nullptr);

    // using mtime if atime is zero
    if (!atime)
        atime = mtime;

    // create directory
    auto status = fs::create(fs::dirname(file));
    if (!status)
        return status;

    // create file if not exist
    // todo created immediately?
    if (!std::ofstream(file.c_str()))
        return fs::status(errno);

    // modify mtime and atime
    struct ::stat st{};

    if (!::stat(file.c_str(), &st))
    {
        ::utimbuf time{};

        time.modtime = mtime;
        time.actime  = atime;

        return !::utime(file.c_str(), &time) ? fs::status() : fs::status(errno);
    }
    else
    {
        return fs::status(errno);
    }
}

fs::status fs::create(const std::string &dir, std::uint16_t mode, bool recursive)
{
    // todo does mode correct?
    if (!mode)
        mode = S_IRWXU | S_IRWXG | S_IRWXO;

    if (!fs::isDir(dir))
    {
        if (recursive)
        {
            auto success = true;
            auto dirname = fs::dirname(dir);

            if (!dirname.empty())
                success = fs::create(dirname, mode, recursive) && success;

            return !::mkdir(dir.c_str(), mode) && success ? fs::status() : fs::status(errno);
        }
        else
        {
            return !::mkdir(dir.c_str(), mode) ? fs::status() : fs::status(errno);
        }
    }
    else
    {
        return fs::status();
    }
}

fs::status fs::remove(const std::string &path)
{
    if (fs::isFile(path))
    {
        return !::remove(path.c_str()) ? fs::status() : fs::status(errno);
    }
    else
    {
        DIR    *dir = ::opendir(path.c_str());
        dirent *cur = nullptr;

        if (!dir)
            return fs::status();  // treat file not found as success

        auto ok  = true;
        auto sep = fs::sep();

        // todo do not recursive?
        // remove sub items
        while ((cur = ::readdir(dir)))
        {
            std::string name(cur->d_name);

            if ((name == ".") || (name == ".."))
                continue;

            std::string full(path.back() == sep.front() ? path + name : path + sep + name);

            if (fs::isDir(full))
                ok = fs::remove(full) && ok;
            else
                ok = !::remove(full.c_str()) && ok;
        }

        ::closedir(dir);

        // remove itself
        if (ok)
            ok = !::rmdir(path.c_str());

        return ok ? fs::status() : fs::status(errno);
    }
}

#endif