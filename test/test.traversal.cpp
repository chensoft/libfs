/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.traversal")
{
    auto tmp = fs::tmp() + fs::sep() + fs::uuid();

    CHECK(fs::mkdir(tmp + "/usr/bin"));
    CHECK(fs::mkdir(tmp + "/usr/lib"));
    CHECK(fs::touch(tmp + "/usr/bin/zip"));
    CHECK(fs::touch(tmp + "/usr/lib/libz.a"));

    std::vector<std::string> children_first = {tmp + "/usr/bin", tmp + "/usr/bin/zip", tmp + "/usr/lib", tmp + "/usr/lib/libz.a"};
    std::vector<std::string> siblings_first = {tmp + "/usr/bin", tmp + "/usr/lib", tmp + "/usr/bin/zip", tmp + "/usr/lib/libz.a"};
    std::vector<std::string> deepest_first  = {tmp + "/usr/bin/zip", tmp + "/usr/bin", tmp + "/usr/lib/libz.a", tmp + "/usr/lib"};

    CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::ChildrenFirst) == children_first);
    CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::SiblingsFirst) == siblings_first);
    CHECK(fs::collect(tmp + "/usr", true, fs::VisitStrategy::DeepestFirst)  == deepest_first);
}