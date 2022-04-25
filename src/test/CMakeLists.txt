# Now simply link against gtest or gtest_main as needed. Eg
add_executable(tests tests.cpp ../Utilities.cpp ../Utilities.h ../Grammar.h ../Grammar.cpp)

include_directories(..)

target_link_libraries(tests gtest_main zlib)

