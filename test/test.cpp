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
        CHECK(fs::sep("/bin") == '/');
        CHECK(fs::sep("C:\\Windows") == '\\');  // support Windows path on Unix
        CHECK(fs::sep("Users\\x/Downloads") == '\\');  // mix '\' and '/' is valid on Windows

        CHECK(fs::drive("C:\\Windows\\System32") == "C:\\");
        CHECK(fs::drive("/usr/local") == "/");
        CHECK(fs::drive("file.txt").empty());
        CHECK(fs::drive("").empty());
    }

    // -------------------------------------------------------------------------
    SECTION("path")
    {

    }

    // -------------------------------------------------------------------------
    SECTION("type")
    {
        CHECK(fs::isAbsolute("/usr/local"));
        CHECK_FALSE(fs::isAbsolute(""));
        CHECK_FALSE(fs::isAbsolute("file.txt"));
        CHECK(fs::isAbsolute("C:\\Windows\\System32"));

        CHECK_FALSE(fs::isRelative("/usr/local"));
        CHECK(fs::isRelative(""));
        CHECK(fs::isRelative("file.txt"));
        CHECK_FALSE(fs::isRelative("C:\\Windows\\System32"));
    }

    // -------------------------------------------------------------------------
    SECTION("property")
    {
        CHECK(fs::atime("/") > 0);
        CHECK(fs::mtime("/") > 0);
        CHECK(fs::ctime("/") > 0);

        auto tmp = fs::tmp() + "/" + fs::rand();

        CHECK(fs::write(tmp + "/file.txt", "abc"));
        CHECK(fs::filesize(tmp + "/file.txt") == 3);
    }

    // -------------------------------------------------------------------------
    SECTION("operate")
    {
        auto tmp = fs::realpath(fs::tmp()) + "/" + fs::rand();
        auto cwd = fs::cwd();
        auto old = std::string();

        CHECK(fs::mkdir(tmp));
        CHECK(fs::change(tmp, &old));
        CHECK(old == cwd);
        CHECK(fs::cwd() == tmp);

        CHECK(fs::touch(tmp + "/file.txt"));
        CHECK(fs::isFile(tmp + "/file.txt"));

        CHECK(fs::mkdir(tmp + "/dir1"));
        CHECK_FALSE(fs::isDir(tmp + "/dir2"));
        CHECK(fs::rename(tmp + "/dir1", tmp + "/dir2"));
        CHECK(fs::isDir(tmp + "/dir2"));
        CHECK_FALSE(fs::isDir(tmp + "/dir1"));
    }
}