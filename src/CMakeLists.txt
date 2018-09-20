add_library(fs "")

# environment
set_target_properties(
    fs PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    VERSION ${libfs_VERSION}
)

target_compile_definitions(
    fs PUBLIC
    FS_VERSION_MAJOR=${libfs_VERSION_MAJOR}
    FS_VERSION_MINOR=${libfs_VERSION_MINOR}
    FS_VERSION_PATCH=${libfs_VERSION_PATCH}
    FS_VERSION="${libfs_VERSION}"
)

# include path
target_include_directories(fs PUBLIC ${PROJECT_SOURCE_DIR}/include)

# source codes
file(GLOB_RECURSE PROJ_INC ${PROJECT_SOURCE_DIR}/include/*.hpp)  # I hate listing all the files
file(GLOB_RECURSE PROJ_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

# static build
# use -DBUILD_SHARED_LIBS=ON to build a shared library
target_sources(fs PRIVATE ${PROJ_INC} ${PROJ_SRC})

# install libs
install(TARGETS fs LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/fs DESTINATION include)

# code warnings
if(UNIX)
    target_compile_options(fs PRIVATE -Wall -Wextra -Wmissing-field-initializers)
endif()

# code coverage
option(ENABLE_CODE_COVERAGE "Enable code coverage." OFF)

if(ENABLE_CODE_COVERAGE)
    target_compile_options(fs PUBLIC -g -O0 --coverage)  # debug, no optimization, enable coverage
    target_link_libraries(fs gcov)
endif()