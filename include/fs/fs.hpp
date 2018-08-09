/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <string>

namespace fs
{
    // -------------------------------------------------------------------------
    // path
    // -------------------------------------------------------------------------

    // Current system drive directory
    // on Windows, return "C:\" in most cases
    // on Unix, return "/" always
    std::string root();

    // Current home directory
    std::string home();

    // Current temp directory
    std::string tmp();

    // Current working directory
    std::string cwd();
}