/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.mode")
{
    CHECK(fs::isReadable(fs::home()));
    CHECK(fs::isWritable(fs::home()));
    CHECK(fs::isExecutable(fs::home()));
}