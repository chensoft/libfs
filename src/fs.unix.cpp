/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
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

// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
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

#endif