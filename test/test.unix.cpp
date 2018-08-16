/**
 * Created by Jian Chen
 * @since  2018.08.16
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#if defined(__unix__) || defined(__APPLE__)

#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.unix")
{
    // -------------------------------------------------------------------------
    SECTION("sys")
    {
        CHECK("/" == fs::root());

        CHECK(!fs::home().empty());
        CHECK(!::unsetenv("HOME"));
        CHECK(!fs::home().empty());
        CHECK(!::setenv("HOME", fs::home().c_str(), 1));

        auto tmp = fs::tmp();

        CHECK(!tmp.empty());
        CHECK(!::unsetenv("TMPDIR"));
        CHECK("/tmp" == fs::tmp());
        CHECK(!::setenv("TMPDIR", tmp.c_str(), 1));

        CHECK(!fs::cwd().empty());

        CHECK('/' == fs::sep());

        CHECK(!fs::drives().empty());
    }
}

#endif