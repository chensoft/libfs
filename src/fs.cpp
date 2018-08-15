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
std::string fs::drive(const std::string &path)
{
    // todo empty if relative path
    // todo / if unix otherwise path.substr(0, 3)
}

// -----------------------------------------------------------------------------
// path
std::string fs::normalize(const std::string &path)
{
    // todo test speed with old version
    // todo celero in one header
//    if (path.empty())
//        return "";

//    std::string ret;
//    std::size_t cur = 0;
//    std::size_t len = path.size();

//    // expand ~ to home directory
//    if (path[cur++] == '~')
//    {
//        ret += fs::home();
//        ret += fs::sep();
//    }
//
//    std::vector<std::pair<std::size_t, std::size_t>> store;  // segments cache
//
//    for (std::size_t i = top.size(), l = path.size(); (i < l) || len; ++i)
//    {
//        if ((i == l) || (path[i] == sep))
//        {
//            if (len > 0)
//            {
//                // store a segment
//                if (str::equal(&path[ptr], len, "..", 2))
//                {
//                    if (!store.empty() && !str::equal(&path[store.back().first], store.back().second, "..", 2))
//                        store.pop_back();
//                    else if (!store.empty() || !abs)
//                        store.emplace_back(std::make_pair(ptr, len));
//                }
//                else if (!str::equal(&path[ptr], len, ".", 1))
//                {
//                    store.emplace_back(std::make_pair(ptr, len));
//                }
//
//                ptr = len = 0;
//            }
//        }
//        else
//        {
//            if (!len)
//            {
//                ptr = i;
//                len = 1;
//            }
//            else
//            {
//                ++len;
//            }
//        }
//    }
//
//    // concat
//    std::string ret(top);
//
//    for (std::size_t i = 0, l = store.size(); i < l; ++i)
//    {
//        auto &pair = store[i];
//        ret.append(&path[pair.first], pair.second);
//
//        if (i < l - 1)
//            ret.append(1, sep);
//    }
//
//    return ret;
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
    if (path.empty())
        return false;

    if (path.front() == '/')
        return true;

    return (path.size() >= 3) && std::isalpha(path[0]) && (path[1] == ':') && (path[2] == '\\');
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