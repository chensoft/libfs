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
    //        auto tmp = fs::tmp() + fs::sep() + fs::uuid();
    //
    //        CHECK(fs::write(tmp + "/file.txt", "abcde"));
    //        CHECK(fs::read(tmp + "/file.txt") == "abcde");
    //
    //        CHECK(fs::append(tmp + "/file.txt", "\n12345"));
    //        CHECK(fs::read(tmp + "/file.txt") == "abcde\n12345");
    //
    //        std::vector<std::string> line_by_line = {"abcde", "12345"};
    //
    //        CHECK(fs::read(tmp + "/file.txt", '\n') == line_by_line);
}