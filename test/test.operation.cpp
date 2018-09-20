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
    auto tmp = fs::realpath(fs::tmp()) + fs::sep() + fs::uuid();
    auto cwd = fs::cwd();
    auto old = std::string();

    // chdir
    CHECK(fs::mkdir(tmp));
    CHECK(fs::chdir(tmp, &old));
    CHECK(fs::cwd() == tmp);
    CHECK(old == cwd);

    // touch
    CHECK(fs::touch("file.txt"));
    CHECK(fs::isFile("file.txt"));

    // mkdir
    CHECK(fs::mkdir(""));
    CHECK(fs::mkdir("a/b/c"));
    CHECK(fs::isDir("a/b/c"));

    // rename
    CHECK(fs::rename("a", "x"));
    CHECK(fs::isDir("x/b/c"));
    CHECK_FALSE(fs::isDir("a"));

    // remove
    CHECK(fs::remove(fs::uuid()));  // non-existent path
    CHECK(fs::remove("x"));
    CHECK_FALSE(fs::isDir("x"));

    // todo copy

    // symlink
    CHECK(fs::mkdir("dir"));
    CHECK(fs::symlink("dir", "link"));
    CHECK(fs::isSymlink("link"));
    CHECK(fs::isDir("link"));
    CHECK_FALSE(fs::isDir("link", false));

    //        // todo test big file copy in release mode
    //        CHECK(fs::write(tmp + "/source/a/b/c/file.txt", "abcde"));
    //        CHECK(fs::filesize(tmp + "/source/a/b/c/file.txt") == 5);
    //        CHECK(fs::copy(tmp + "/source", tmp + "/target"));  // todo need modify
    //        CHECK(fs::filesize(tmp + "/target/a/b/c/file.txt") == 5);
}