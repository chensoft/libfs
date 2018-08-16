/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <system_error>
#include <functional>
#include <string>
#include <vector>
#include <ctime>

namespace fs
{
    // -------------------------------------------------------------------------
    // status
    // -------------------------------------------------------------------------
    class status
    {
    public:
        explicit status(int code = 0) : error(code, std::generic_category()) {}
        explicit operator bool() const { return !error; }

        std::error_code error;
    };


    // -------------------------------------------------------------------------
    // sys
    // -------------------------------------------------------------------------

    // Current system drive directory
    // @result "C:\" on Windows in most cases, "/" on Unix always
    std::string root();

    // Current home directory
    std::string home();

    // Current temp directory
    std::string tmp();

    // Current working directory
    std::string cwd();

    // Separator on this platform
    // @result '/' on Unix, '\' on Windows
    char sep();

    // Separator based on the path
    // @result '/' or '\' when one of them was first found
    // @note support both Unix & Windows path on any platform
    char sep(const std::string &path);

    // Get system drives
    // @result "C:\", "D:\" ... on Windows, "/" on Unix
    std::vector<std::string> drives();

    // Get path's drive
    // e.g: C:\Windows\System32 -> "C:\"
    // e.g: /usr/local -> "/"
    // e.g: file.txt -> ""
    // @note support both Unix & Windows path on any platform
    std::string drive(const std::string &path);

    // -------------------------------------------------------------------------
    // path
    // -------------------------------------------------------------------------

    // Expand all symbolic links, remove ".", ".." and redundant separators
    // it will treat the beginning '~' as current user's home directory
    // it will append relative path to the current working path
    std::string realpath(const std::string &path);

    // Normalize path, remove ".", ".." and redundant separators, but does not follow the symbolic link
    // it will treat the beginning '~' as current user's home directory
    // Unix:
    // e.g: ./a -> a, a/./b -> a/b, a///b -> a/b
    // e.g: a/.../b -> a/.../b because the "..." is invalid path characters, it will be ignored
    // e.g: a/../../b -> ../b because the path is relative and second ".." can't be removed
    // e.g: /usr/local/etc/.. -> /usr/local
    // e.g: /.. -> /
    // Windows:
    // e.g: C:\a -> C:\a, C:\.\a -> C:\a
    // e.g: C:\a\...\b -> C:\a\...\b
    // e.g: C:\a\..\..\b -> C:\b
    // e.g: C:\a\..\b -> C:\b
    // @note support both Unix & Windows path on any platform
    std::string normalize(const std::string &path);  // todo some func's string can not be an constant

    // Directory name of the path, without the trailing slash
    // Unix:
    // e.g: /home/staff/Downloads/file.txt -> "/home/staff/Downloads"
    // e.g: /usr/. -> "/usr", because "." is represent current directory
    // e.g: /usr/ -> "/", not "/usr", because single "/" isn't a effective name
    // e.g: /usr/// -> "/", because the trailing slash will be ignored
    // e.g: / -> "/", because it's already the root directory
    // e.g: file.txt -> ".", because it's a relative path
    // Windows:
    // e.g: C:\Windows\System32 -> "C:\Windows"
    // e.g: C:\Windows\System32\cmd.exe -> "C:\Windows\System32"
    // e.g: C:\\\ -> C:\
    // e.g: C:\ -> C:\
    // @note support both Unix & Windows path on any platform
    std::string dirname(const std::string &path);

    // Base name of the path, if you provide suffix, it will be removed from result
    // Unix:
    // e.g: /home/staff/Downloads/file.txt -> "file.txt"
    // e.g: /home/ -> "home"
    // e.g: / -> ""
    // e.g: file.txt -> "file.txt"
    // Windows:
    // e.g: C:\Windows\System32\cmd.exe -> "cmd.exe"
    // e.g: C:\ -> ""
    // @note support both Unix & Windows path on any platform
    std::string basename(const std::string &path, bool with_ext = true);

    // Extension name of the path
    // Unix:
    // e.g: /home/staff/Downloads/file.txt -> ".txt"
    // e.g: /home/ -> ""
    // e.g: / -> ""
    // Windows:
    // e.g: C:\Windows\System32\cmd.exe -> ".exe"
    // e.g: C:\ -> ""
    // @note support both Unix & Windows path on any platform
    std::string extname(const std::string &path, bool with_dot = true);

    // -------------------------------------------------------------------------
    // exist
    // -------------------------------------------------------------------------

    // Check if the file or directory exist
    bool isExist(const std::string &path);

    // Check if the path is a directory
    bool isDir(const std::string &path, bool follow_symlink = true);

    // Check if the path is a regular file
    bool isFile(const std::string &path, bool follow_symlink = true);

    // Check if the path is a symbolic link
    bool isLink(const std::string &path);

    // -------------------------------------------------------------------------
    // type
    // -------------------------------------------------------------------------

    // Check if the path is an absolute path
    // @note support both Unix & Windows path on any platform
    bool isAbsolute(const std::string &path);

    // Check if the path is a relative path
    // @note support both Unix & Windows path on any platform
    bool isRelative(const std::string &path);

    // -------------------------------------------------------------------------
    // mode
    // -------------------------------------------------------------------------

    // Check if the file or directory is readable
    bool isReadable(const std::string &path);

    // Check if the file or directory is writable
    bool isWritable(const std::string &path);

    // Check if the file or directory is executable
    bool isExecutable(const std::string &path);

    // -------------------------------------------------------------------------
    // property
    // -------------------------------------------------------------------------

    // Get file or directory's access time
    // todo high dpi timestamp?
    std::time_t atime(const std::string &path);

    // Get file or directory's modification time
    std::time_t mtime(const std::string &path);

    // Get file or directory's create time
    std::time_t ctime(const std::string &path);

    // Get file size
    std::size_t filesize(const std::string &file);

    // -------------------------------------------------------------------------
    // operate
    // -------------------------------------------------------------------------

    // Change current working directory
    status change(const std::string &path_new, std::string *path_old = nullptr);

    // Set access and modification time of the file, create file and its dir if it's not exist
    // @param file the file to be access or create
    // @param mtime modification time, if zero then use current time
    // @param atime access time, if zero then use mtime
    status touch(const std::string &file, std::time_t mtime = 0, std::time_t atime = 0);

    // Create a directory
    // @param mode default mode is rwxr-xr-x
    // @param recursive recursively or not
    // todo how does mode represent on Windows S_IRWXU xxx
    status create(const std::string &dir, std::uint16_t mode = 0, bool recursive = true);

    // Rename a file or directory
    status rename(const std::string &path_old, const std::string &path_new);

    // Remove a file or directory
    status remove(const std::string &path);

    // Copy a file or directory
    status copy(const std::string &path_old, const std::string &path_new);

    // -------------------------------------------------------------------------
    // visit
    // -------------------------------------------------------------------------

    // Visit the directory items use depth-first traversal, exclude '.' and '..'
    // todo distinct dir vs path vs file
    void visit(const std::string &dir,
               std::function<void (const std::string &path)> callback,
               bool recursive = true);
    void visit(const std::string &dir,
               std::function<void (const std::string &path, bool *stop)> callback,
               bool recursive = true);

    // Collect all items in the directory, exclude '.' and '..'
    std::vector<std::string> collect(const std::string &directory,
                                     bool recursive = true);

    // -------------------------------------------------------------------------
    // IO
    // -------------------------------------------------------------------------

    // Read all file contents to a string
    std::string read(const std::string &file);

    // Read part of file's contents to a string
    std::string read(const std::string &file, std::streamoff start, std::streamoff length);

    // Read file line by line using delimiter
    std::vector<std::string> read(const std::string &file, char delimiter);

    // Write data to the file
    status write(const std::string &file, const std::string &data);
    status write(const std::string &file, const void *data, std::size_t size);

    status append(const std::string &file, const std::string &data);
    status append(const std::string &file, const void *data, std::size_t size);
}