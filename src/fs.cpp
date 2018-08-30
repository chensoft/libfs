/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include <sys/stat.h>
#include <fstream>
#include <codecvt>
#include <memory>
#include <random>
#include <locale>
#include <cctype>

// -----------------------------------------------------------------------------
// helper
std::wstring fs::widen(const std::string &utf8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(utf8);
}

std::string fs::narrow(const std::wstring &utf16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(utf16);
}

std::string fs::prune(std::string dir)
{
    auto beg = dir.c_str();
    auto ptr = beg + dir.size() - 1;

    while (ptr >= beg && (*ptr == '/' || *ptr == '\\'))
        --ptr;

    return dir.resize(ptr - beg + 1), dir;
}

// -----------------------------------------------------------------------------
// path
std::string fs::uuid()
{
    std::string pattern("xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx");
    std::random_device device;
    auto threshold = (std::random_device::max)();

    for (auto &c : pattern)
    {
        if ((c == '-') || (c == '4'))
            continue;

        auto r = static_cast<int>((double)device() / threshold * 16);
        auto v = c == 'x' ? r : ((r & 0x03) | 0x08);

        c = static_cast<char>(v <= 9 ? '0' + v : 'A' - 10 + v);
    }

    return pattern;
}

char fs::sep(const std::string &path)
{
    auto pos = path.find_first_of("/\\");
    return pos != std::string::npos ? path[pos] : fs::sep();
}

std::string fs::seps()
{
    return "/\\";
}

std::string fs::drive(const std::string &path)
{
    if (path.empty())
        return "";

    if (path.front() == '/')
        return "/";

    return path.size() >= 3 && std::isalpha(path[0]) && path[1] == ':' && path[2] == '\\' ? path.substr(0, 1) : "";
}

// -----------------------------------------------------------------------------
// split
std::string fs::normalize(std::string path)
{
    // todo test speed with old version
    // todo allow mix windows and unix style
    auto ret = std::string();
//    auto tok = fs::tokenize(fs::expand(std::move(path)));
//
//    for (auto &pair : tok)
//    {
//        // ignore "."
//        if (pair.second == ".")
//            continue;
//
//        // backtrace ".."
//        if (pair.second == "..")
//        {
//            auto pos = ret.find_last_of(fs::seps());
//            ret.resize(pos == std::string::npos ? 0 : (std::max)((decltype(pos))1, pos));  // preserve first '/'
//            continue;
//        }
//
//        ret += pair.first;
//        ret += pair.second;
//    }

    return ret;
}

std::string fs::expand(std::string path)
{
    auto ptr = path.c_str();
    return *ptr++ == '~' && (*ptr == '/' || *ptr == '\\' || !*ptr) ? path.replace(0, 1, fs::home()), path : path;
}

std::vector<std::string> fs::tokenize(const std::string &path, const std::string &seps)
{
    std::vector<std::string> ret;

    const char *beg = path.c_str();
    const char *end = nullptr;
    const char *cur = beg;

    for (; *cur; cur = end)
    {
        // skip duplicate separators
        while (seps.find(*cur) != std::string::npos)
            ++cur;

        // locate end pointer
        for (end = cur; *end && seps.find(*end) == std::string::npos; ++end)
            ;

        // add new separator
        if (cur == beg + 1 || (cur > beg && end != cur))
            ret.emplace_back(cur - 1, cur);

        // add new component
        if (end != cur)
            ret.emplace_back(cur, end);
    }

    return ret;
}

//std::string fs::dirname(const std::string &path)
//{
//    // todo optimize
//    // todo how to handle drive 'C:\Windows'
//    auto pos = path.find_last_of("/\\");
//    return pos != std::string::npos ? path.substr(0, pos) : "";
//}
//
//std::string fs::basename(const std::string &path, bool with_ext)
//{
//    auto pos = path.find_last_of("/\\");
//    auto ext = !with_ext ? fs::extname(path) : "";
//    return pos != std::string::npos ? path.substr(pos + 1, !ext.empty() ? path.size() - pos - 1 - ext.size() : std::string::npos) : path;  // todo use *c_str instead empty
//}
//
//std::string fs::extname(const std::string &path, bool with_dot)
//{
//    auto pos = path.find_last_of('.');
//    return pos != std::string::npos ? path.substr(with_dot ? pos : pos + 1) : "";
//}
//
//// -----------------------------------------------------------------------------
//// type
//bool fs::isAbsolute(const std::string &path)
//{
//    return !fs::drive(path).empty();
//}
//
//bool fs::isRelative(const std::string &path)
//{
//    return !fs::isAbsolute(path);
//}
//
//// -----------------------------------------------------------------------------
//// operation
//fs::status fs::rename(const std::string &path_old, const std::string &path_new)
//{
//    // remove existence path
//    auto result = fs::remove(path_new);
//    if (!result)
//        return result;
//
//    // create new directory
//    result = fs::mkdir(fs::dirname(path_new));
//    if (!result)
//        return result;
//
//    return !::rename(path_old.c_str(), path_new.c_str()) ? status() : status(errno);
//}
//
//fs::status fs::copy(const std::string &path_old, const std::string &path_new)
//{
//    if (fs::isDir(path_old, false))
//    {
//        auto result = fs::mkdir(path_new);
//        if (!result)
//            return result;
//
//        auto size = path_old.size();
//
//        fs::visit(path_old, [&] (const std::string &path, bool *stop) {
//            auto sub = path.substr(size, path.size() - size);
//
//            if (fs::isFile(path))
//                result = fs::copy(path, path_new + sub);
//            else
//                result = fs::mkdir(path_new + sub);
//
//            *stop = !result;
//        });
//
//        return result;
//    }
//    else
//    {
//        auto result = fs::mkdir(fs::dirname(path_new));
//        if (!result)
//            return result;
//
//        auto deleter = [] (FILE *ptr) { ::fclose(ptr); };
//
//        // todo use setbuf(f, NULL) disable buffering
//        // todo use mmap, std async
//        std::unique_ptr<FILE, decltype(deleter)> in(::fopen(path_old.c_str(), "rb"), deleter);
//        std::unique_ptr<FILE, decltype(deleter)> out(::fopen(path_new.c_str(), "wb"), deleter);
//
//        if (!in or !out)
//            return status(errno);
//
//        char buf[4096];
//        size_t len = 0;
//
//        while ((len = ::fread(buf, 1, sizeof(buf), in.get())) > 0)
//        {
//            if (::fwrite(buf, 1, len, out.get()) != len)
//                return status(::ferror(out.get()));
//        }
//
//        return status(::ferror(in.get()));
//    }
//}
//
//// -----------------------------------------------------------------------------
//// traversal
//void fs::visit(const std::string &dir, const std::function<void (const std::string &path)> &callback, bool recursive, VisitStrategy strategy)
//{
//    fs::visit(dir, [&] (const std::string &path, bool *) {
//        callback(path);
//    }, recursive, strategy);
//}
//
//// -----------------------------------------------------------------------------
//// IO
//std::string fs::read(const std::string &file)
//{
//    return fs::read(file, 0, fs::filesize(file));
//}
//
//std::string fs::read(const std::string &file, std::size_t start, std::size_t length)
//{
//    if (length <= 0)
//        return "";
//
//    std::ifstream in(file, std::ios_base::binary);
//
//    if (start)
//        in.seekg(start);
//
//    if (in && !in.eof())
//    {
//        std::string ret(length, '\0');
//        in.read(&ret[0], static_cast<std::streamsize>(length));
//        ret.resize(static_cast<std::size_t>(in.gcount()));
//
//        return ret;
//    }
//
//    return "";
//}
//
//std::vector<std::string> fs::read(const std::string &file, char sep)
//{
//    // todo use fopen
//    std::vector<std::string> ret;
//    std::ifstream in(file, std::ios_base::binary);
//    std::string line;
//
//    while (std::getline(in, line, sep))
//        ret.emplace_back(std::move(line));
//
//    return ret;
//}
//
//// write
//fs::status fs::write(const std::string &file, const std::string &data)
//{
//    return fs::write(file, data.data(), data.size());
//}
//
//fs::status fs::write(const std::string &file, const void *data, std::size_t size)
//{
//    if (!fs::mkdir(fs::dirname(file)))
//        return status(errno);
//
//    std::ofstream out(file, std::ios_base::binary);
//    if (out)
//        out.write(static_cast<const char*>(data), size);
//
//    return out.good() ? status() : status(errno);
//}
//
//fs::status fs::append(const std::string &file, const std::string &data)
//{
//    return fs::append(file, data.data(), data.size());
//}
//
//fs::status fs::append(const std::string &file, const void *data, std::size_t size)
//{
//    if (!fs::mkdir(fs::dirname(file)))
//        return status(errno);
//
//    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
//    if (out)
//        out.write(static_cast<const char*>(data), size);
//
//    return out.good() ? status() : status(errno);
//}