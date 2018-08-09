/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include <unistd.h>
#include <limits.h>
#include <pwd.h>

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
    char buf[PATH_MAX + 1];
    auto ret = ::getcwd(buf, sizeof(buf));
    return ret ? ret : "";
}

#endif