/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.utils")
{
    CHECK(std::string(u8"\u9648\u5251") == "\xE9\x99\x88\xE5\x89\x91");
    CHECK(fs::widen(u8"\u9648\u5251") == L"\u9648\u5251");
    CHECK(fs::narrow(L"\u9648\u5251") == "\xE9\x99\x88\xE5\x89\x91");

    CHECK(fs::prune("").empty());
    CHECK(fs::prune("/") == "/");
    CHECK(fs::prune("//") == "/");
    CHECK(fs::prune("/usr/local") == "/usr/local");
    CHECK(fs::prune("/usr/local/") == "/usr/local");
    CHECK(fs::prune("/usr/local//") == "/usr/local");

    CHECK(fs::prune("C:\\") == "C:\\");
    CHECK(fs::prune("C:\\\\") == "C:\\");
    CHECK(fs::prune("C:\\Windows") == "C:\\Windows");
    CHECK(fs::prune("C:\\Windows\\") == "C:\\Windows");
    CHECK(fs::prune("C:\\Windows\\\\") == "C:\\Windows");
}