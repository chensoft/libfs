/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.io")
{
    auto root = fs::tmp() + fs::sep() + fs::uuid() + fs::sep();

    CHECK(fs::write(root + "file.txt", "abcde"));

    CHECK(fs::read(root + "file.txt") == "abcde");
    CHECK(fs::read(root + "file.txt", 3, 2) == "de");

    CHECK(fs::append(root + "file.txt", "-12345"));

    CHECK(fs::read(root + "file.txt") == "abcde-12345");
    CHECK(fs::read(root + "file.txt", '-') == std::vector<std::string>{"abcde", "12345"});
}