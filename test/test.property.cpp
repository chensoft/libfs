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
    //        auto tmp = fs::tmp() + fs::sep() + fs::uuid() + "/file.txt";  // todo check others do not write multiple file names
    //
    //        CHECK(fs::write(tmp, "abc"));
    //
    //        CHECK(fs::touch(tmp, 0, 12345678));
    //        CHECK(fs::atime(tmp).tv_sec != 12345678);
    //        CHECK(fs::mtime(tmp).tv_sec == 12345678);
    //
    //        CHECK(fs::touch(tmp, 87654321, 12345678));
    //        CHECK(fs::atime(tmp).tv_sec == 87654321);
    //        CHECK(fs::mtime(tmp).tv_sec == 12345678);
    //        CHECK(fs::ctime(tmp).tv_sec > 0);
    //
    //        CHECK(fs::filesize(tmp) == 3);
}