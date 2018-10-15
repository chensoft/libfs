/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.check")
{
    // create
    auto root = fs::tmp() + fs::sep() + fs::uuid() + fs::sep();

    CHECK(fs::mkdir(root + "dir-real"));
    CHECK(fs::touch(root + "file-real"));
    CHECK(fs::touch(root + "file-temp"));
    CHECK(fs::remove(root + "file-temp"));

    // exist
    CHECK(fs::isExist(root + "dir-real"));
    CHECK(fs::isExist(root + "dir-real", false));
    CHECK(fs::isExist(root + "file-real"));
    CHECK(fs::isExist(root + "file-real", false));
    CHECK_FALSE(fs::isExist(root + "file-temp"));
    CHECK_FALSE(fs::isExist(root + "file-temp", false));

    // empty
    CHECK(fs::isEmpty(root + "file-temp"));
    CHECK(fs::isEmpty(root + "file-real"));
    CHECK(fs::write(root + "file-real", "abc"));
    CHECK_FALSE(fs::isEmpty(root + "file-real"));

    CHECK(fs::touch(root + "dir-real/file-inside"));
    CHECK_FALSE(fs::isEmpty(root + "dir-real"));

    // type
    CHECK_FALSE(fs::isDir(root + "file-real"));
    CHECK_FALSE(fs::isDir(root + "file-real", false));

    CHECK_FALSE(fs::isFile(root + "dir-real"));
    CHECK_FALSE(fs::isFile(root + "dir-real", false));

    CHECK_FALSE(fs::isSymlink(root + "dir-real"));
    CHECK_FALSE(fs::isSymlink(root + "file-real"));
}