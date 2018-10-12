add_executable(fs-test)

# environment
set_target_properties(
    fs-test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# source codes
file(GLOB_RECURSE PROJ_INC *.hpp)
file(GLOB_RECURSE PROJ_SRC *.cpp)

# generate app
target_sources(fs-test PRIVATE ${PROJ_INC} ${PROJ_SRC})

# link library
target_link_libraries(fs-test fs)