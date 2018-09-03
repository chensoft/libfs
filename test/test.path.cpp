/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.path")
{
    SECTION("all")
    {
        CHECK(!fs::root().empty());
        CHECK(!fs::user().empty());
        CHECK(fs::home(fs::user()) == fs::home());
        CHECK(fs::home().back() != fs::sep());
        CHECK(fs::tmp().back() != fs::sep());
        CHECK(fs::cwd().back() != fs::sep());
        CHECK(!fs::uuid().empty());

        CHECK(fs::sep());
        CHECK(fs::seps() == "/\\");

        CHECK(!fs::drives().empty());

        CHECK(fs::drive("C:\\Windows\\System32") == "C:\\");
        CHECK(fs::drive("/usr/local") == "/");
        CHECK(fs::drive("file.txt").empty());
        CHECK(fs::drive("").empty());
    }

    SECTION("unix")
    {
#if defined(__unix__) || defined(__APPLE__)
        CHECK(fs::root() == "/");

        CHECK(!fs::home().empty());
        CHECK(!::unsetenv("HOME"));
        CHECK(!fs::home().empty());
        CHECK(!::setenv("HOME", fs::home().c_str(), 1));

        auto tmp = fs::tmp();

        CHECK(!tmp.empty());
        CHECK(!::unsetenv("TMPDIR"));
        CHECK(fs::tmp() == "/tmp");
        CHECK(!::setenv("TMPDIR", tmp.c_str(), 1));
#endif
    }
}