/**
 * Created by Jian Chen
 * @since  2018.08.16
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"
#include <dirent.h>

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
        CHECK(fs::atime(fs::root()) > 0);
        CHECK(fs::mtime(fs::root()) > 0);
        CHECK(fs::ctime(fs::root()) > 0);

        auto tmp = fs::tmp() + fs::sep() + fs::rand();

        CHECK(fs::write(tmp + "/file.txt", "abc"));
        CHECK(fs::filesize(tmp + "/file.txt") == 3);
    }

    // -------------------------------------------------------------------------
    SECTION("operate")
    {
        auto tmp = fs::realpath(fs::tmp()) + fs::sep() + fs::rand();
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

        CHECK(fs::remove(tmp + fs::sep() + fs::rand()));  // non-existent path is ok
        CHECK(fs::isDir(tmp + "/dir2"));
        CHECK(fs::remove(tmp + "/dir2"));
        CHECK_FALSE(fs::isDir(tmp + "/dir2"));

        CHECK(fs::write(tmp + "/source/a/b/c/file.txt", "abcde"));
        CHECK(fs::filesize(tmp + "/source/a/b/c/file.txt") == 5);
        CHECK(fs::copy(tmp + "/source", tmp + "/target"));  // todo need modify
        CHECK(fs::filesize(tmp + "/target/a/b/c/file.txt") == 5);

        CHECK(fs::symlink(tmp + "/source", tmp + "/link"));
        CHECK(fs::isSymlink(tmp + "/link"));
    }

    // -------------------------------------------------------------------------
    SECTION("operate")
    {
        auto tmp = fs::tmp() + fs::sep() + fs::rand();

        CHECK(fs::mkdir(tmp + "/usr/bin"));
        CHECK(fs::mkdir(tmp + "/usr/lib"));
        CHECK(fs::touch(tmp + "/usr/bin/zip"));
        CHECK(fs::touch(tmp + "/usr/lib/libz.a"));

        std::vector<std::string> children_first = {tmp + "/usr/bin", tmp + "/usr/bin/zip", tmp + "/usr/lib", tmp + "/usr/lib/libz.a"};
        std::vector<std::string> siblings_first = {tmp + "/usr/bin", tmp + "/usr/lib", tmp + "/usr/bin/zip", tmp + "/usr/lib/libz.a"};
        std::vector<std::string> deepest_first  = {tmp + "/usr/bin/zip", tmp + "/usr/bin", tmp + "/usr/lib/libz.a", tmp + "/usr/lib"};

        CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::ChildrenFirst) == children_first);
        CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::SiblingsFirst) == siblings_first);
        CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::DeepestFirst)  == deepest_first);
    }
}