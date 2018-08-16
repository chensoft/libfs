/**
 * Created by Jian Chen
 * @since  2018.08.16
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs")
{
    // -------------------------------------------------------------------------
    SECTION("sys")
    {
        CHECK(fs::sep() == fs::sep("bin"));
        CHECK('/' == fs::sep("/bin"));
        CHECK('\\' == fs::sep("C:\\Windows"));  // support Windows path on Unix
        CHECK('\\' == fs::sep("Users\\x/Downloads"));  // mix '\' and '/' is valid on Windows

        CHECK("C:\\" == fs::drive("C:\\Windows\\System32"));
        CHECK("/" == fs::drive("/usr/local"));
        CHECK("" == fs::drive("file.txt"));
        CHECK("" == fs::drive(""));
    }
}