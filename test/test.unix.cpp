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
    }

//    // -------------------------------------------------------------------------
//    SECTION("check")
//    {
//        // todo can move to test.cpp
//        {
//            auto tmp = fs::tmp() + "/" + fs::uuid();
//
//            CHECK(fs::mkdir(tmp));
//            CHECK(fs::mkdir(tmp + "/folder"));
//            CHECK(fs::touch(tmp + "/file.txt"));
//            CHECK(fs::symlink(tmp + "/file.txt", tmp + "/exist.txt"));
//            CHECK(fs::symlink(tmp + "/folder", tmp + "/directory"));
//            CHECK(fs::touch(tmp + "/dummy.txt"));
//            CHECK(fs::symlink(tmp + "/dummy.txt", tmp + "/broken.txt"));
//            CHECK(fs::remove(tmp + "/dummy.txt"));
//
//            CHECK(fs::isExist(tmp));
//            CHECK(fs::isExist(tmp, false));
//            CHECK(fs::isExist(tmp + "/file.txt"));
//            CHECK(fs::isExist(tmp + "/file.txt", false));
//            CHECK(fs::isExist(tmp + "/exist.txt"));
//            CHECK(fs::isExist(tmp + "/exist.txt", false));
//            CHECK_FALSE(fs::isExist(tmp + "/broken.txt"));
//            CHECK(fs::isExist(tmp + "/broken.txt", false));
//
//            CHECK(fs::isDir(tmp));
//            CHECK(fs::isDir(tmp, false));
//            CHECK(fs::isDir(tmp + "/directory"));
//            CHECK_FALSE(fs::isDir(tmp + "/directory", false));
//            CHECK_FALSE(fs::isDir(tmp + "/file.txt"));
//            CHECK_FALSE(fs::isDir(tmp + "/file.txt", false));
//
//            CHECK_FALSE(fs::isFile(tmp));
//            CHECK(fs::isFile(tmp + "/file.txt"));
//            CHECK(fs::isFile(tmp + "/exist.txt"));
//            CHECK_FALSE(fs::isFile(tmp + "/exist.txt", false));
//
//            CHECK_FALSE(fs::isSymlink("tmp"));
//            CHECK(fs::isSymlink(tmp + "/exist.txt"));
//            CHECK_FALSE(fs::isSymlink(tmp + "/file.txt"));
//        }
//
//        {
//            auto tmp = fs::tmp() + "/" + fs::uuid();
//
//            CHECK(fs::isEmpty(tmp));  // path is not exist now
//            CHECK(fs::mkdir(tmp));
//            CHECK(fs::isEmpty(tmp));  // path has no entries
//            CHECK(fs::touch(tmp + "/file.txt"));
//            CHECK_FALSE(fs::isEmpty(tmp));
//            CHECK(fs::isEmpty(tmp + "/file.txt"));
//            CHECK(fs::write(tmp + "/file.txt", "abc"));
//            CHECK_FALSE(fs::isEmpty(tmp + "/file.txt"));
//        }
//    }
//
//    // -------------------------------------------------------------------------
//    SECTION("mode")
//    {
//        fs::isReadable("/");
//        fs::isWritable("/");
//        fs::isExecutable("/");
//    }
}

#endif