/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.property")
{
    auto file = fs::tmp() + fs::sep() + fs::uuid() + fs::sep() + "file.txt";

    CHECK(fs::write(file, "abc"));

    auto time = fs::ctime(file);

    CHECK(fs::touch(file, 0, 12345678));
    CHECK(fs::atime(file).tv_sec != 12345678);
    CHECK(fs::mtime(file).tv_sec == 12345678);
    CHECK(fs::ctime(file).tv_sec == time.tv_sec);

    CHECK(fs::touch(file, 87654321, 12345678));
    CHECK(fs::atime(file).tv_sec == 87654321);
    CHECK(fs::mtime(file).tv_sec == 12345678);
    CHECK(fs::ctime(file).tv_sec == time.tv_sec);

    CHECK(fs::filesize(file) == 3);
}