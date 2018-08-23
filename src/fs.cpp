/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include <sys/stat.h>
#include <fstream>
#include <memory>
#include <cctype>

// -----------------------------------------------------------------------------
// sys
char fs::sep(const std::string &path)
{
    auto pos = path.find_first_of("/\\");
    return pos != std::string::npos ? path[pos] : fs::sep();
}

std::string fs::drive(const std::string &path)
{
    if (path.empty())
        return "";

    if (path.front() == '/')
        return "/";

    return (path.size() >= 3) && std::isalpha(path[0]) && (path[1] == ':') && (path[2] == '\\') ? path.substr(0, 3) : "";
}

// -----------------------------------------------------------------------------
// path
std::string fs::normalize(const std::string &path)
{
    // todo test speed with old version
    // todo celero in one header
    // todo use ptr == '\0' check string end
    std::string ret;

    const char *cur = path.c_str();
    const char *end = cur;

    // expand '~' to home path
    if ((*cur == '~') && (!*(cur + 1) || (*(cur + 1) == '/') || (*(cur + 1) == '\\')))
    {
        ret += fs::home();

        if (*++cur)
            ret += *cur;
    }

    for (; *cur; cur = end)
    {
        // skip redundant separators
        while ((*cur == '/') || (*cur == '\\'))
            ++cur;

        // locate end pointer
        for (end = cur; *end && (*end != '/') && (*end != '\\'); ++end)
            ;

        if (end - cur == 0)
            break;

        // ignore '.'
        if ((end - cur == 1) && (cur[0] == '.'))
            continue;

        if ((end - cur == 2) && (cur[0] == '.') && (cur[1] == '.'))
        {
            // backtrace if find '..'
            auto pos = ret.find_last_of("/\\");
            if (pos != std::string::npos)
                ret.resize(pos);
        }
        else
        {
            // add new component
            ret.append(cur, end);
        }
    }

    return ret;
}

std::string fs::dirname(const std::string &path)
{
    // todo how to handle drive 'C:\Windows'
    auto pos = path.find_last_of("/\\");
    return pos != std::string::npos ? path.substr(0, pos) : "";
}

std::string fs::basename(const std::string &path, bool with_ext)
{
    auto pos = path.find_last_of("/\\");
    auto ext = !with_ext ? fs::extname(path) : "";
    return pos != std::string::npos ? path.substr(pos + 1, !ext.empty() ? path.size() - pos - 1 - ext.size() : std::string::npos) : path;
}

std::string fs::extname(const std::string &path, bool with_dot)
{
    auto pos = path.find_last_of('.');
    return pos != std::string::npos ? path.substr(with_dot ? pos : pos + 1) : "";
}

// -----------------------------------------------------------------------------
// type
bool fs::isAbsolute(const std::string &path)
{
    return !fs::drive(path).empty();
}

bool fs::isRelative(const std::string &path)
{
    return !fs::isAbsolute(path);
}

// -----------------------------------------------------------------------------
// operate
fs::status fs::rename(const std::string &source, const std::string &target)
{
    // remove existence path
    auto result = fs::remove(target);
    if (!result)
        return result;

    // create new directory
    result = fs::mkdir(fs::dirname(target));
    if (!result)
        return result;

    return !::rename(source.c_str(), target.c_str()) ? status() : status(errno);
}

fs::status fs::copy(const std::string &source, const std::string &target)
{
    if (fs::isDir(source, false))
    {
        auto result = fs::mkdir(target);
        if (!result)
            return result;

        auto size = source.size();

        fs::visit(source, [&] (const std::string &path, bool *stop) {
            auto sub = path.substr(size, path.size() - size);

            if (fs::isFile(path))
                result = fs::copy(path, target + sub);
            else
                result = fs::mkdir(target + sub);

            *stop = !result;
        });

        return result;
    }
    else
    {
        auto result = fs::mkdir(fs::dirname(target));
        if (!result)
            return result;

        auto deleter = [] (FILE *ptr) { ::fclose(ptr); };

        // todo use setbuf(f, NULL) disable buffering
        // todo use mmap, std async
        std::unique_ptr<FILE, decltype(deleter)> in(::fopen(source.c_str(), "rb"), deleter);
        std::unique_ptr<FILE, decltype(deleter)> out(::fopen(target.c_str(), "wb"), deleter);

        if (!in or !out)
            return status(errno);

        char buf[4096];
        size_t len = 0;

        while ((len = ::fread(buf, 1, sizeof(buf), in.get())) > 0)
        {
            if (::fwrite(buf, 1, len, out.get()) != len)
                return status(::ferror(out.get()));
        }

        return status(::ferror(in.get()));
    }
}

// -----------------------------------------------------------------------------
// visit
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
    if (length <= 0)
        return "";

    std::ifstream in(file, std::ios_base::binary);

    if (start)
        in.seekg(start);

    if (in && !in.eof())
    {
        std::string ret(length, '\0');
        in.read(&ret[0], static_cast<std::streamsize>(length));
        ret.resize(static_cast<std::size_t>(in.gcount()));

        return ret;
    }

    return "";
}

std::vector<std::string> fs::read(const std::string &file, char delimiter)
{
    // todo use fopen
    std::vector<std::string> ret;
    std::ifstream in(file, std::ios_base::binary);
    std::string line;

    while (std::getline(in, line, delimiter))
        ret.emplace_back(std::move(line));

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

    std::ofstream out(file, std::ios_base::binary);
    if (out)
        out.write(static_cast<const char*>(data), size);

    return out.good() ? status() : status(errno);
}

fs::status fs::append(const std::string &file, const std::string &data)
{
    return fs::append(file, data.data(), data.size());
}

fs::status fs::append(const std::string &file, const void *data, std::size_t size)
{
    if (!fs::mkdir(fs::dirname(file)))
        return status(errno);

    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
    if (out)
        out.write(static_cast<const char*>(data), size);

    return out.good() ? status() : status(errno);
}