/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 * @note   We use utf-8 encoding on all platforms
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
    // utils
    // -------------------------------------------------------------------------

    // utf-8 to wstring
    std::wstring widen(const std::string &utf8);

    // wstring to utf-8
    std::string narrow(const std::wstring &wstr);

    // Remove the separator at the end, preserve drive
    std::string prune(std::string dir, const std::string &drv = "");

    // -------------------------------------------------------------------------
    // path
    // -------------------------------------------------------------------------

    // Retrieve system root path
    // @result "/" on Unix always, "C:\" on Windows in most cases
    std::string root();

    // Retrieve current user
    std::string user();

    // Retrieve home directory, no separator at the end
    std::string home();
    std::string home(const std::string &user);

    // Retrieve temp directory, no separator at the end
    std::string tmp();

    // Current working directory, no separator at the end
    std::string cwd();

    // Generate a uuid string
    std::string uuid();

    // Separator on current system
    // @result '/' on Unix, '\' on Windows
    char sep();

    // Separator on all platforms
    std::string seps();

    // Get mount points or system drives
    // @result "/" on Unix, "C", "D" ... on Windows
    std::vector<std::string> drives();

    // Get path's mount point or drive
    // e.g: "" -> ""
    // e.g: "file.txt" -> ""
    // e.g: "/usr/local" -> "/"
    // e.g: "C:\Windows\System32" -> "C:\"
    // @note support both Unix & Windows path on any platform
    std::string drive(const std::string &path);  // todo consider return std size_t, most place doesn't need string

    // -------------------------------------------------------------------------
    // split
    // -------------------------------------------------------------------------

    // Absolute path in the filesystem, expand all symbolic links
    // *) will expand the beginning '~'
    // *) will change relative path to absolute
    // *) will remove ".", ".." and duplicate separators
    std::string realpath(std::string path);

    // Normalize the path, does not expand the symbolic link
    // *) will expand the beginning '~'
    // *) will remove ".", ".." and duplicate separators
    // Unix:
    // e.g: "./a" -> "a", "a/." -> "a", "a/./b" -> "a/b", "a///b" -> "a/b"
    // e.g: "a/.../b" -> "a/.../b" because the "..." is invalid path characters, it will be ignored
    // e.g: "a/../../b" -> "../b" because the path is relative and second ".." can't be removed
    // e.g: "a/../b" -> "b"
    // e.g: "a/b/.." -> "a"
    // e.g: "/.." -> "/"
    // Windows:
    // e.g: "C:\a" -> "C:\a", "C:\.\a" -> "C:\a"
    // e.g: "C:\a\...\b" -> "C:\a\...\b"
    // e.g: "C:\a\..\..\b" -> "C:\b"
    // e.g: "C:\a\..\b" -> "C:\b"
    // @note support both Unix & Windows path on any platform
    std::string normalize(std::string path);

    // Expand ~ to current home directory
    // e.g: "" -> ""
    // e.g: "~" -> fs::home()
    // e.g: "~/go" -> fs::home() + "/go"
    // e.g: "~xxx" -> "~xxx" because ~ is part of the name
    std::string expand(std::string path);

    // todo expand variables

    // Tokenize the path into multiple segments, the first item is always the drive letter
    // *) will use empty string if no drives
    // *) will skip the duplicate separators
    // Unix:
    // e.g: "" -> ("", "")
    // e.g: "/" -> ("/", "")
    // e.g: "usr" -> ("", ""), ("usr", "") first item empty means it's a relative path
    // e.g: "/usr" -> ("/", ""), ("usr", "")
    // e.g: "/usr/bin" -> ("/", ""), ("usr", "/"), ("bin", "")
    // e.g: "/usr/bin/" -> ("/", ""), ("usr", "/"), ("bin", "/") the last '/' is preserved
    // e.g: "/usr///bin"  -> ("/", ""), ("usr", "/"), ("bin", "")
    // Windows:
    // e.g: "C:\" -> ("C:\", "")
    // e.g: "C:\Windows" -> ("C:\", ""), ("Windows", "")
    // Mix:
    // e.g: "C:\Windows/System32" -> ("C:\", ""), ("Windows", "/"), ("System32", "")
    // e.g: "C:\Windows\/System32" -> ("C:\", ""), ("Windows", "\"), ("System32", "") the second separator is '\'
    // @note support both Unix & Windows path on any platform
    void tokenize(const std::string &path, const std::function<void (std::string component, char separator)> &callback);

    // Directory name of the path, without the trailing slash
    // Unix:
    // e.g: "." -> ""
    // e.g: "./usr" -> "."
    // e.g: "/" -> "/", because it's already the root directory
    // e.g: "//" -> "/", because it's already the root directory
    // e.g: "/usr" -> "/"
    // e.g: "/usr/" -> "/", not "/usr", because single "/" isn't a effective name
    // e.g: "/usr/." -> "/usr"
    // e.g: "/usr///" -> "/", because the trailing slash will be ignored
    // e.g: "/home/staff/Downloads/file.txt" -> "/home/staff/Downloads"
    // Windows:
    // e.g: "C:\" -> "C:\"
    // e.g: "C:\\" -> "C:\"
    // e.g: "C:\Windows\System32" -> "C:\Windows"
    // e.g: "C:\Windows\System32\cmd.exe" -> "C:\Windows\System32"
    // @note support both Unix & Windows path on any platform
    std::string dirname(const std::string &path);

    // Base name of the path
    // Unix:
    // e.g: "file.txt" -> "file.txt"
    // e.g: "/" -> ""
    // e.g: "//" -> ""
    // e.g: "/home/" -> "home"
    // e.g: "/home/staff/Downloads/file.txt" -> "file.txt"
    // Windows:
    // e.g: "C:\" -> ""
    // e.g: "C:\\" -> ""
    // e.g: "C:\Windows\System32\cmd.exe" -> "cmd.exe"
    // @note support both Unix & Windows path on any platform
    std::string basename(const std::string &path, bool with_ext = true);

    // Extension name of the path
    // Unix:
    // e.g: "file.txt" -> ".txt"
    // e.g: "/" -> ""
    // e.g: "/home/" -> ""
    // e.g: "/home/staff/Downloads/file.txt" -> ".txt"
    // Windows:
    // e.g: "C:\" -> ""
    // e.g: "C:\Windows\System32\cmd.exe" -> ".exe"
    // @note support both Unix & Windows path on any platform
    std::string extname(const std::string &path, bool with_dot = true);

    // -------------------------------------------------------------------------
    // check
    // -------------------------------------------------------------------------

    // Check if the file or directory exist
    bool isExist(const std::string &path, bool follow_symlink = true);

    // Check if the file has contents or directory has entries
    // @note non-existent path will be considered empty too
    bool isEmpty(const std::string &path);

    // Check if the path is a directory
    bool isDir(const std::string &path, bool follow_symlink = true);

    // Check if the path is a regular file
    bool isFile(const std::string &path, bool follow_symlink = true);

    // Check if the path is a symbolic link
    bool isSymlink(const std::string &path);

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
    struct ::timespec atime(const std::string &path);

    // Get file or directory's modification time
    struct ::timespec mtime(const std::string &path);

    // Get file or directory's create time
    // todo ctime is not create time?
    struct ::timespec ctime(const std::string &path);

    // Get file size
    std::size_t filesize(const std::string &file);

    // -------------------------------------------------------------------------
    // operation
    // -------------------------------------------------------------------------

    // Change current working directory
    status chdir(const std::string &dir_new, std::string *dir_old = nullptr);

    // Create file if not exist and change its timestamps
    // @param file the file to be access or create
    // @param atime access time, if zero then use current time
    // @param mtime modification time, if zero then use current time
    // @note only support second precision now because high precision require newer os
    status touch(const std::string &file, std::time_t atime = 0, std::time_t mtime = 0);

    // Create a directory
    // @param mode default mode is rwxr-xr-x
    // @note empty dir will be considered successful
    // todo check all relative paths
    status mkdir(const std::string &dir, std::uint16_t mode = 0755);

    // Rename a file or directory
    status rename(const std::string &path_old, const std::string &path_new);

    // Remove a file or directory
    // @note non-existent path will be considered successful
    status remove(const std::string &path);

    // todo add trash method

    // Copy a file or directory
    // todo if target exist and is dir then copy source file to target folder
    // todo if target not exist, treat it as directory or target file path?
    // todo if source is directory, should copy itself to target
    // todo parameter need re-design
    status copy(const std::string &path_old, const std::string &path_new);

    // Symbol link file or directory
    // @param path the original object
    // @param link the symbolic link
    status symlink(const std::string &path, const std::string &link);

    // -------------------------------------------------------------------------
    // traversal
    // -------------------------------------------------------------------------

    // Visit the directory items use different traversal methods, exclude '.' and '..'
    // @e.g: children-first: /usr/bin, /usr/bin/zip, /usr/lib, /usr/lib/libz.a
    // @e.g: siblings-first: /usr/bin, /usr/lib, /usr/bin/zip, /usr/lib/libz.a
    // @e.g: deepest-first: /usr/bin/zip, /usr/bin, /usr/lib/libz.a, /usr/lib
    enum class VisitStrategy { ChildrenFirst, SiblingsFirst, DeepestFirst };

    // todo visit use template and return type F same as the lambda
    // todo visit should return relative path
    // todo visit should not follow symlink
    // todo !!!visit callback accept a PathEntry struct, has field: root, name, deep, provide follow_symlink parameter, use enum option?
    // todo all visit and collect rename to walk
    // todo do not provide callback, return vector directly
    void visit(const std::string &dir, const std::function<void (const std::string &path)> &callback, bool recursive = true, VisitStrategy strategy = VisitStrategy::ChildrenFirst);
    void visit(const std::string &dir, const std::function<void (const std::string &path, bool *stop)> &callback, bool recursive = true, VisitStrategy strategy = VisitStrategy::ChildrenFirst);

    // Collect all items in the directory, exclude '.' and '..'
    // todo provide parameter return 'absolute' or relative path
    std::vector<std::string> collect(const std::string &dir, bool recursive = true, VisitStrategy strategy = VisitStrategy::ChildrenFirst);

    // todo provide find function, search files even by regex, like 'find' utility, support unix glob style * ? [] [!]

    // -------------------------------------------------------------------------
    // IO
    // -------------------------------------------------------------------------

    // todo fstream is too slow, use others
    // Read all file contents to a string
    std::string read(const std::string &file);

    // Read part of file's contents to a string
    std::string read(const std::string &file, std::size_t start, std::size_t length);

    // Read file line by line using separator
    std::vector<std::string> read(const std::string &file, char sep);

    // Write data to the file
    status write(const std::string &file, const std::string &data);
    status write(const std::string &file, const void *data, std::size_t size);

    // Append data to the file
    status append(const std::string &file, const std::string &data);
    status append(const std::string &file, const void *data, std::size_t size);

    // todo add resize, resize file to specific size
}