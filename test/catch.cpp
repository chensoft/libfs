/**
 * Created by Jian Chen
 * @since  2018.08.06
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#define CATCH_CONFIG_RUNNER

#include "fs/fs.hpp"
#include "catch.hpp"

int main(int argc, char *argv[])
{
    Catch::Session().run(argc, argv);
	::system("pause");
}