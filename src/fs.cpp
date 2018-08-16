/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#include "fs/fs.hpp"
#include <sys/stat.h>
#include <fstream>

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
// property
std::time_t fs::atime(const std::string &path)
{
    struct ::stat st{};
    return !::stat(path.c_str(), &st) ? st.st_atime : 0;
}

std::time_t fs::mtime(const std::string &path)
{
    struct ::stat st{};
    return !::stat(path.c_str(), &st) ? st.st_mtime : 0;
}

std::time_t fs::ctime(const std::string &path)
{
    struct ::stat st{};
    return !::stat(path.c_str(), &st) ? st.st_ctime : 0;
}

std::size_t fs::filesize(const std::string &file)
{
    struct ::stat st{};
    return !::stat(file.c_str(), &st) ? static_cast<std::size_t>(st.st_size) : 0;
}

// -----------------------------------------------------------------------------
// visit
void fs::visit(const std::string &dir,
               std::function<void (const std::string &path)> callback,
               bool recursive)
{
    fs::visit(dir, [&] (const std::string &path, bool *stop) {
        *stop = false;
        callback(path);
    }, recursive);
}

// -----------------------------------------------------------------------------
// IO
std::string fs::read(const std::string &file)
{
    return fs::read(file, 0, fs::filesize(file));
}

std::string fs::read(const std::string &file, std::streamoff start, std::streamoff length)
{
    if (length <= 0)
        return "";

    std::ifstream in(file, std::ios_base::binary);

    if (start)
        in.seekg(start);

    if (in && !in.eof())
    {
        std::string ret(static_cast<std::size_t>(length), '\0');
        in.read(&ret[0], static_cast<std::streamsize>(length));
        ret.resize(static_cast<std::size_t>(in.gcount()));

        return ret;
    }

    return "";
}

std::vector<std::string> fs::read(const std::string &file, char delimiter)
{
    // todo delimiter unused
    std::vector<std::string> ret;
    std::ifstream in(file, std::ios_base::binary);
    std::string line;

    while (std::getline(in, line))
        ret.emplace_back(std::move(line));

    return ret;
}

// write
fs::status fs::write(const std::string &file, const std::string &data)
{
    if (fs::create(fs::dirname(file)))
        return status(errno);

    std::ofstream out(file, std::ios_base::binary);
    if (out)
        out.write(data.data(), data.size());

    return out.good() ? fs::status() : status(errno);
}

fs::status fs::write(const std::string &file, const void *data, std::size_t size)
{
    if (fs::create(fs::dirname(file)))
        return status(errno);

    std::ofstream out(file, std::ios_base::binary);
    if (out)
        out.write(static_cast<const char*>(data), size);

    return out.good() ? fs::status() : status(errno);
}

fs::status fs::append(const std::string &file, const std::string &data)
{
    if (fs::create(fs::dirname(file)))
        return status(errno);

    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
    if (out)
        out.write(data.data(), data.size());

    return out.good() ? fs::status() : status(errno);
}

fs::status fs::append(const std::string &file, const void *data, std::size_t size)
{
    if (fs::create(fs::dirname(file)))
        return status(errno);

    std::ofstream out(file, std::ios_base::binary | std::ios_base::app);
    if (out)
        out.write(static_cast<const char*>(data), size);

    return out.good() ? fs::status() : status(errno);
}