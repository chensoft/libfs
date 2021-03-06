/**
 * Created by Jian Chen
 * @since  2018.09.03
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include "catch.hpp"

TEST_CASE("fs.split")
{
    SECTION("realpath")
    {
        CHECK(!fs::realpath("~").empty());
        CHECK(fs::realpath("~") == fs::realpath(fs::home()));
        CHECK(fs::realpath("relative") == fs::realpath(fs::cwd() + fs::sep() + "relative"));  // cwd maybe a symbolic link
    }

    SECTION("normalize")
    {
        CHECK(fs::normalize("").empty());
        CHECK(fs::normalize("~") == fs::home());
        CHECK(fs::normalize("./a") == "a");
        CHECK(fs::normalize("a/.") == "a");
        CHECK(fs::normalize("a/./b") == "a/b");
        CHECK(fs::normalize("a///b") == "a/b");
        CHECK(fs::normalize("a/.../b") == "a/.../b");  // this is a invalid path
        CHECK(fs::normalize("a/../../b") == "../b");
        CHECK(fs::normalize("a/b/..") == "a");
        CHECK(fs::normalize("a/../b") == "b");
        CHECK(fs::normalize("/..") == "/");

        CHECK(fs::normalize("C:\\a") == "C:\\a");
        CHECK(fs::normalize("C:\\.\\a") == "C:\\a");
        CHECK(fs::normalize("C:\\a\\...\\b") == "C:\\a\\...\\b");
        CHECK(fs::normalize("C:\\a\\..\\..\\b") == "C:\\b");
        CHECK(fs::normalize("C:\\a\\..\\b") == "C:\\b");
    }

    SECTION("expand")
    {
        CHECK(fs::expand("").empty());
        CHECK(fs::expand("~") == fs::home());
        CHECK(fs::expand("~/go") == fs::home() + "/go");
        CHECK(fs::expand("~xxx") == "~xxx");
    }

    SECTION("tokenize")
    {
        typedef std::vector<std::string> tokenize_type;

        auto tokenize = [](const std::string &path) {
            tokenize_type ret;
            fs::tokenize(path, [&](std::string component, char separator) {
                ret.emplace_back(std::move(component));
                ret.emplace_back(separator ? 1 : 0, separator);
            });
            return ret;
        };

        CHECK(tokenize("") == tokenize_type({"", ""}));
        CHECK(tokenize("/") == tokenize_type({"/", ""}));
        CHECK(tokenize("usr") == tokenize_type({"", "", "usr", ""}));
        CHECK(tokenize("/usr") == tokenize_type({"/", "", "usr", ""}));
        CHECK(tokenize("/usr/bin") == tokenize_type({"/", "", "usr", "/", "bin", ""}));
        CHECK(tokenize("/usr/bin/") == tokenize_type({"/", "", "usr", "/", "bin", "/"}));
        CHECK(tokenize("/usr///bin") == tokenize_type({"/", "", "usr", "/", "bin", ""}));
        CHECK(tokenize("C:\\") == tokenize_type({"C:\\", ""}));
        CHECK(tokenize("C:\\Windows") == tokenize_type({"C:\\", "", "Windows", ""}));
        CHECK(tokenize("C:\\Windows/System32") == tokenize_type({"C:\\", "", "Windows", "/", "System32", ""}));
        CHECK(tokenize("C:\\Windows\\/System32") == tokenize_type({"C:\\", "", "Windows", "\\", "System32", ""}));
    }

    SECTION("dirname")
    {
        CHECK(fs::dirname(".").empty());
        CHECK(fs::dirname("./usr") == ".");
        CHECK(fs::dirname("/") == "/");
        CHECK(fs::dirname("//") == "/");
        CHECK(fs::dirname("/usr") == "/");
        CHECK(fs::dirname("/usr/") == "/");
        CHECK(fs::dirname("/usr/.") == "/usr");
        CHECK(fs::dirname("/usr///") == "/");
        CHECK(fs::dirname("/home/staff/Downloads/file.txt") == "/home/staff/Downloads");

        CHECK(fs::dirname("C:\\") == "C:\\");
        CHECK(fs::dirname("C:\\\\") == "C:\\");
        CHECK(fs::dirname("C:\\Windows\\System32") == "C:\\Windows");
        CHECK(fs::dirname("C:\\Windows\\System32\\cmd.exe") == "C:\\Windows\\System32");
    }

    SECTION("basename")
    {
        // with ext
        CHECK(fs::basename("file") == "file");
        CHECK(fs::basename("file.txt") == "file.txt");

        CHECK(fs::basename("/").empty());
        CHECK(fs::basename("//").empty());
        CHECK(fs::basename("/home/") == "home");
        CHECK(fs::basename("/home/staff/vm.box/debian") == "debian");
        CHECK(fs::basename("/home/staff/Downloads/file.txt") == "file.txt");

        CHECK(fs::basename("C:\\").empty());
        CHECK(fs::basename("C:\\\\").empty());
        CHECK(fs::basename("C:\\Windows\\System32\\cmd.exe") == "cmd.exe");

        // without ext
        CHECK(fs::basename("file", false) == "file");
        CHECK(fs::basename("file.txt", false) == "file");

        CHECK(fs::basename("/", false).empty());
        CHECK(fs::basename("//", false).empty());
        CHECK(fs::basename("/home/", false) == "home");
        CHECK(fs::basename("/home/staff/vm.box/debian", false) == "debian");
        CHECK(fs::basename("/home/staff/Downloads/file.txt", false) == "file");

        CHECK(fs::basename("C:\\", false).empty());
        CHECK(fs::basename("C:\\\\", false).empty());
        CHECK(fs::basename("C:\\Windows\\System32\\cmd.exe", false) == "cmd");
    }

    SECTION("extname")
    {
        // with dot
        CHECK(fs::extname("file").empty());
        CHECK(fs::extname("file.txt") == ".txt");

        CHECK(fs::extname("/").empty());
        CHECK(fs::extname("/home/").empty());
        CHECK(fs::extname("/home/staff/vm.box/debian").empty());
        CHECK(fs::extname("/home/staff/Downloads/file.txt") == ".txt");

        CHECK(fs::extname("C:\\").empty());
        CHECK(fs::extname("C:\\Windows\\System32\\cmd.exe") == ".exe");

        // without dot
        CHECK(fs::extname("file", false).empty());
        CHECK(fs::extname("file.txt", false) == "txt");

        CHECK(fs::extname("/", false).empty());
        CHECK(fs::extname("/home/", false).empty());
        CHECK(fs::extname("/home/staff/vm.box/debian", false).empty());
        CHECK(fs::extname("/home/staff/Downloads/file.txt", false) == "txt");

        CHECK(fs::extname("C:\\", false).empty());
        CHECK(fs::extname("C:\\Windows\\System32\\cmd.exe", false) == "exe");
    }
}