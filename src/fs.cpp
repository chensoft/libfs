/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include <algorithm>
#include <codecvt>
#include <memory>
#include <random>
#include <locale>
#include <cctype>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4996)  // fopen
#endif

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
    auto len = !drv.empty() ? drv.size() : fs::drive(dir);

    dir.erase(std::find_if(dir.rbegin(), dir.rend() - len, [=] (char c) {
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

    fs::tokenize(fs::expand(std::move(path)), [&] (std::string component, char separator) {
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
    auto end = std::find_if(path.rbegin(), path.rend() - drv, [] (char c) { return fs::seps().find(c) == std::string::npos; });
    auto cur = std::find_if(end, path.rend() - drv, [] (char c) { return fs::seps().find(c) != std::string::npos; });
    return std::string(path.begin(), cur != path.rend() - drv ? cur.base() - 1 : path.begin() + drv);
}

std::string fs::basename(const std::string &path, bool with_ext)
{
    auto drv = fs::drive(path);
    auto end = std::find_if(path.rbegin(), path.rend() - drv, [] (char c) { return fs::seps().find(c) == std::string::npos; });
    auto cur = std::find_if(end, path.rend() - drv, [] (char c) { return fs::seps().find(c) != std::string::npos; });
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
fs::status fs::rename(const std::string &path_old, const std::string &path_new)
{
    // remove existence path
    auto result = fs::remove(path_new);
    if (!result)
        return result;

    // create new directory
    result = fs::mkdir(fs::dirname(path_new));
    if (!result)
        return result;

    return !::rename(path_old.c_str(), path_new.c_str()) ? status() : status(errno);  // todo check errno in Windows, and can use in error_code?
}

fs::status fs::remove(const std::string &path)
{
    // todo check errno on Windows
    if (!::remove(path.c_str()) || errno == ENOENT)
        return {};

    if (errno != ENOTEMPTY)
        return status(errno);

    auto error = 0;

    fs::visit(path, [&](const std::string &item, bool *stop) {
        if (::remove(item.c_str()))
        {
            *stop = true;
            error = errno;
        }
    }, true, VisitStrategy::DeepestFirst);

    return !error && ::remove(path.c_str()) ? status(errno) : status(error);
}

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

        fs::visit(source, [&] (const std::string &path, bool *stop) {
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
// traversal
void fs::visit(const std::string &dir, const std::function<void (const std::string &path)> &callback, bool recursive, VisitStrategy strategy)
{
    fs::visit(dir, [&] (const std::string &path, bool *) {
        callback(path);
    }, recursive, strategy);
}

// -----------------------------------------------------------------------------
// IO
std::string fs::read(const std::string &file)
{
    return fs::read(file, 0, fs::filesize(file));
}

std::string fs::read(const std::string &file, std::size_t start, std::size_t length)
{
    auto deleter = [] (FILE *ptr) { ::fclose(ptr); };
    std::unique_ptr<FILE, decltype(deleter)> in(::fopen(file.c_str(), "rb"), deleter);

    if (!in || (start && ::fseek(in.get(), start, SEEK_SET)))
        return "";

    std::string ret(length, '\0');
    ret.resize(::fread(&ret[0], 1, length, in.get()));
    return ret;
}

// write
fs::status fs::write(const std::string &file, const std::string &data)
{
    return fs::write(file, data.data(), data.size());
}

fs::status fs::write(const std::string &file, const void *data, std::size_t size)
{
    if (!fs::mkdir(fs::dirname(file)))
        return status(errno);

    auto deleter = [] (FILE *ptr) { ::fclose(ptr); };
    std::unique_ptr<FILE, decltype(deleter)> out(::fopen(file.c_str(), "wb"), deleter);
    if (!out)
        return status(errno);

    return ::fwrite(data, 1, size, out.get()) == size ? status() : status(errno);
}

fs::status fs::append(const std::string &file, const std::string &data)
{
    return fs::append(file, data.data(), data.size());
}

fs::status fs::append(const std::string &file, const void *data, std::size_t size)
{
    if (!fs::mkdir(fs::dirname(file)))
        return status(errno);

    auto deleter = [] (FILE *ptr) { ::fclose(ptr); };
    std::unique_ptr<FILE, decltype(deleter)> out(::fopen(file.c_str(), "ab+"), deleter);
    if (!out)
        return status(errno);

    return ::fwrite(data, 1, size, out.get()) == size ? status() : status(errno);
}

#ifdef _WIN32
    #pragma warning(pop)
#endif