/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.operation")
{
    //        auto tmp = fs::realpath(fs::tmp()) + fs::sep() + fs::uuid();
    //        auto cwd = fs::cwd();
    //        auto old = std::string();
    //
    //        CHECK(fs::mkdir(tmp));
    //        CHECK(fs::chdir(tmp, &old));
    //        CHECK(old == cwd);
    //        CHECK(fs::cwd() == tmp);
    //
    //        CHECK(fs::touch(tmp + "/file.txt"));
    //        CHECK(fs::isFile(tmp + "/file.txt"));
    //
    //        CHECK(fs::mkdir(tmp + "/dir1"));
    //        CHECK_FALSE(fs::isDir(tmp + "/dir2"));
    //        CHECK(fs::rename(tmp + "/dir1", tmp + "/dir2"));
    //        CHECK(fs::isDir(tmp + "/dir2"));
    //        CHECK_FALSE(fs::isDir(tmp + "/dir1"));
    //
    //        CHECK(fs::remove(tmp + fs::sep() + fs::uuid()));  // non-existent path is ok
    //        CHECK(fs::isDir(tmp + "/dir2"));
    //        CHECK(fs::remove(tmp + "/dir2"));
    //        CHECK_FALSE(fs::isDir(tmp + "/dir2"));
    //
    //        // todo test big file copy in release mode
    //        CHECK(fs::write(tmp + "/source/a/b/c/file.txt", "abcde"));
    //        CHECK(fs::filesize(tmp + "/source/a/b/c/file.txt") == 5);
    //        CHECK(fs::copy(tmp + "/source", tmp + "/target"));  // todo need modify
    //        CHECK(fs::filesize(tmp + "/target/a/b/c/file.txt") == 5);
    //
    //        CHECK(fs::symlink(tmp + "/source", tmp + "/link"));
    //        CHECK(fs::isSymlink(tmp + "/link"));
}