/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include <algorithm>
#include <fstream>
#include <codecvt>
#include <random>
#include <locale>
#include <cctype>

// -----------------------------------------------------------------------------
// utils
std::wstring fs::widen(const std::string &utf8)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(utf8);
}

std::string fs::narrow(const std::wstring &wstr)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

std::string fs::prune(std::string dir, const std::string &drv)
{
    dir.erase(std::find_if(dir.rbegin(), dir.rend() - (!drv.empty() ? drv.size() : fs::drive(dir)), [=](char c) {
        return fs::seps().find(c) == std::string::npos;
    }).base(), dir.end());
    return dir;
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
        if (c == '-' || c == '4')
            continue;

        auto r = static_cast<int>((double)device() / threshold * 16);
        auto v = c == 'x' ? r : ((r & 0x03) | 0x08);

        c = static_cast<char>(v <= 9 ? '0' + v : 'A' - 10 + v);
    }

    return pattern;
}

std::string fs::seps()
{
    return "/\\";
}

std::size_t fs::drive(const std::string &path)
{
    if (path.empty())
        return 0;

    if (path.front() == '/')
        return 1;

    return path.size() >= 3 && std::isalpha(path[0]) && path[1] == ':' && path[2] == '\\' ? 3 : 0;
}

// -----------------------------------------------------------------------------
// split
std::string fs::normalize(std::string path)
{
    std::string ret;
    std::string drv;

    bool drive = false;

    fs::tokenize(fs::expand(std::move(path)), [&](std::string component, char separator) {
        // add drive letter
        if (!drive)
        {
            drive = true;
            ret = drv = component;
            return;
        }

        // ignore "."
        if (component == ".")
            return;

        // backtrace ".."
        if (component == ".." && !ret.empty())
        {
            // can not exceed drive
            if (ret == drv)
                return;

            // resize the final ret
            auto pos = ret.find_last_of(fs::seps(), ret.size() - 2);
            ret.resize(pos == std::string::npos ? 0 : pos + 1);

            return;
        }

        // add segment
        ret += component;

        if (separator)
            ret += separator;
    });

    return fs::prune(std::move(ret), drv);
}

std::string fs::expand(std::string path)
{
    auto ptr = path.c_str();
    return *ptr++ == '~' && (fs::seps().find(*ptr) != std::string::npos || !*ptr) ? path.replace(0, 1, fs::home()), path : path;
}

void fs::tokenize(const std::string &path, const std::function<void (std::string component, char separator)> &callback)
{
    std::size_t drive(fs::drive(path));
    std::string delim(fs::seps());

    // handle drive letter first
    callback(path.substr(0, drive), '\0');

    const char *beg = path.c_str() + drive;
    const char *end = nullptr;
    const char *cur = beg;

    for (; *cur; cur = end)
    {
        // skip duplicate separators
        while (delim.find(*cur) != std::string::npos)
            ++cur;

        // locate end pointer
        for (end = cur; *end && delim.find(*end) == std::string::npos; ++end)
            ;

        if (end == cur)
            break;

        // invoke the callback
        callback(std::string(cur, end), *end ? *end : (char)'\0');
    }
}

std::string fs::dirname(const std::string &path)
{
    auto drv = fs::drive(path);
    auto end = std::find_if(path.rbegin(), path.rend() - drv, [](char c) { return fs::seps().find(c) == std::string::npos; });
    auto cur = std::find_if(end, path.rend() - drv, [](char c) { return fs::seps().find(c) != std::string::npos; });
    return std::string(path.begin(), cur != path.rend() - drv ? cur.base() - 1 : path.begin() + drv);
}

std::string fs::basename(const std::string &path, bool with_ext)
{
    auto drv = fs::drive(path);
    auto end = std::find_if(path.rbegin(), path.rend() - drv, [](char c) { return fs::seps().find(c) == std::string::npos; });
    auto cur = std::find_if(end, path.rend() - drv, [](char c) { return fs::seps().find(c) != std::string::npos; });
    auto ext = with_ext ? cur : std::find(end, cur, '.');
    return std::string(cur.base(), ext == cur ? end.base() : ext.base() - 1);
}

std::string fs::extname(const std::string &path, bool with_dot)
{
    auto pos = path.find_last_of("." + fs::seps());
    return pos != std::string::npos && path[pos] == '.' ? path.substr(with_dot ? pos : pos + 1) : "";
}

// -----------------------------------------------------------------------------
// type
bool fs::isAbsolute(const std::string &path)
{
    return fs::drive(path) > 0;
}

bool fs::isRelative(const std::string &path)
{
    return !fs::drive(path);
}

// -----------------------------------------------------------------------------
// operation
fs::status fs::copy(const std::string &source, std::string target)
{
    // append source's basename if target is a directory
    if (fs::isDir(target))
        target += fs::sep() + fs::basename(source);

    // if source is a directory
    if (fs::isDir(source, false))
    {
        auto result = fs::mkdir(target);
        if (!result)
            return result;

        fs::walk(source, [&](const std::string &path, bool *stop) {
            result = fs::copy(path, target);
            *stop  = !result;
        }, false);

        return result;
    }

    auto result = fs::mkdir(fs::dirname(target));
    if (!result)
        return result;

    // if source is a file
    if (fs::isFile(source, false))
        return fs::write(target, fs::read(source));

    return status(std::errc::not_supported);
}

// -----------------------------------------------------------------------------
// visit
void fs::walk(const std::string &dir, const std::function<void (const std::string &path)> &callback, bool recursive, WalkStrategy strategy)
{
    fs::walk(dir, [&](const std::string &path, bool *) {
        callback(path);
    }, recursive, strategy);
}

std::vector<std::string> fs::find(const std::string &dir, bool recursive, WalkStrategy strategy)
{
    std::vector<std::string> ret;

    fs::walk(dir, [&](const std::string &path) {
        ret.emplace_back(path);
    }, recursive, strategy);

    return ret;
}

// -----------------------------------------------------------------------------
// IO
std::string fs::read(const std::string &file)
{
    return fs::read(file, 0, fs::filesize(file));
}

std::string fs::read(const std::string &file, std::size_t start, std::size_t length)
{
    std::ifstream in(file, std::ios_base::binary);

    if (!in || (start && !in.seekg(start)) || !length)
        return "";

    std::string ret(length, '\0');
    in.read(&ret[0], static_cast<std::streamsize>(length));
    ret.resize(static_cast<std::size_t>(in.gcount()));

    return ret;
}

// write
fs::status fs::write(const std::string &file, const std::string &data)
{
    return fs::write(file, data.data(), data.size());
}

fs::status fs::write(const std::string &file, const void *data, std::size_t size)
{
    auto result = fs::mkdir(fs::dirname(file));
    if (!result)
        return result;

    std::ofstream out(file, std::ios_base::binary);
    if (!out)
        return status(errno);

    return out.write(static_cast<const char*>(data), size) ? status() : status(errno);
}

fs::status fs::append(const std::string &file, const std::string &data)
{
    return fs::append(file, data.data(), data.size());
}

fs::status fs::append(const std::string &file, const void *data, std::size_t size)
{
    auto result = fs::mkdir(fs::dirname(file));
    if (!result)
        return result;

    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
    if (!out)
        return status(errno);

    return out.write(static_cast<const char*>(data), size) ? status() : status(errno);
}