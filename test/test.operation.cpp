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
    CHECK(fs::mkdir("/"));
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
    CHECK(fs::mkdir("x/b/c"));  // for later use

    // symlink
    CHECK(fs::mkdir("dir"));
    CHECK_FALSE(fs::isSymlink("dir"));

    // copy
    CHECK(fs::copy("x", "y"));
    CHECK(fs::copy("file.txt", "copy.txt"));
    CHECK(fs::isDir("y", false));
    CHECK(fs::isFile("copy.txt", false));
}