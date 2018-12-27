/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.visit")
{
    auto tmp = fs::tmp() + fs::sep() + fs::uuid();

    CHECK(fs::mkdir(tmp + "/usr/bin"));
    CHECK(fs::mkdir(tmp + "/usr/lib"));
    CHECK(fs::touch(tmp + "/usr/bin/zip"));
    CHECK(fs::touch(tmp + "/usr/lib/libz.a"));

    auto uni = [](std::string path) {
        std::replace_if(path.begin(), path.end(), [](char c) { return fs::seps().find(c) != std::string::npos; }, fs::sep());
        return path;
    };

    std::vector<std::string> children_first_asc = {tmp + uni("/usr/bin"), tmp + uni("/usr/bin/zip"), tmp + uni("/usr/lib"), tmp + uni("/usr/lib/libz.a")};
    std::vector<std::string> siblings_first_asc = {tmp + uni("/usr/bin"), tmp + uni("/usr/lib"), tmp + uni("/usr/bin/zip"), tmp + uni("/usr/lib/libz.a")};
    std::vector<std::string> deepest_first_asc  = {tmp + uni("/usr/bin/zip"), tmp + uni("/usr/bin"), tmp + uni("/usr/lib/libz.a"), tmp + uni("/usr/lib")};

    std::vector<std::string> children_first_desc = {tmp + uni("/usr/lib"), tmp + uni("/usr/lib/libz.a"), tmp + uni("/usr/bin"), tmp + uni("/usr/bin/zip")};
    std::vector<std::string> siblings_first_desc = {tmp + uni("/usr/lib"), tmp + uni("/usr/bin"), tmp + uni("/usr/lib/libz.a"), tmp + uni("/usr/bin/zip")};
    std::vector<std::string> deepest_first_desc  = {tmp + uni("/usr/lib/libz.a"), tmp + uni("/usr/lib"), tmp + uni("/usr/bin/zip"), tmp + uni("/usr/bin")};

    std::vector<std::string> children_first = fs::find(tmp + uni("/usr"), true, fs::WalkStrategy::ChildrenFirst);
    std::vector<std::string> siblings_first = fs::find(tmp + uni("/usr"), true, fs::WalkStrategy::SiblingsFirst);
    std::vector<std::string> deepest_first  = fs::find(tmp + uni("/usr"), true, fs::WalkStrategy::DeepestFirst);

    CHECK((children_first == children_first_asc || children_first == children_first_desc));
    CHECK((siblings_first == siblings_first_asc || siblings_first == siblings_first_desc));
    CHECK((deepest_first  == deepest_first_asc  || deepest_first  == deepest_first_desc));
}