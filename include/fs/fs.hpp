/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <string>
#include <vector>

namespace fs
{
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
    std::string sep();

    // Get system drives
    // @result "C:\", "D:\" ... on Windows, "/" on Unix
    std::vector<std::string> drives();

    // Get path's drive
    // e.g: C:\Windows\System32 -> "C:\"
    // e.g: /usr/local -> "/"
    // e.g: file.txt -> ""
    // @caution support both Unix & Windows path on any platform
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
    // @caution support both Unix & Windows path on any platform
    std::string normalize(const std::string &path);

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
    // @caution support both Unix & Windows path on any platform
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
    // @caution support both Unix & Windows path on any platform
    std::string basename(const std::string &path, bool with_ext = true);

    // Extension name of the path
    // Unix:
    // e.g: /home/staff/Downloads/file.txt -> ".txt"
    // e.g: /home/ -> ""
    // e.g: / -> ""
    // Windows:
    // e.g: C:\Windows\System32\cmd.exe -> ".exe"
    // e.g: C:\ -> ""
    // @caution support both Unix & Windows path on any platform
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
    // @caution support both Unix & Windows path on any platform
    bool isAbsolute(const std::string &path);

    // Check if the path is a relative path
    // @caution support both Unix & Windows path on any platform
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
}