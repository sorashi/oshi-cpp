cmake_minimum_required(VERSION 3.18)
project(oshi-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_Declare(
        pugixml
        URL https://github.com/zeux/pugixml/releases/download/v1.12/pugixml-1.12.zip
)
FetchContent_Declare(
        jmdict
        URL ftp://ftp.edrdg.org/pub/Nihongo//JMdict_e.gz # yes, two slashes
        DOWNLOAD_NO_EXTRACT true
)
FetchContent_Declare(
        zlib
        URL https://zlib.net/zlib-1.2.12.tar.gz
        URL_HASH SHA256=91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest pugixml jmdict zlib)

enable_testing()

add_subdirectory(test)

include_directories(include)

add_executable(oshi main.cpp Grammar.cpp Grammar.h Utilities.cpp Utilities.h Dictionary.cpp Dictionary.h GrammarFormGuesser.cpp GrammarFormGuesser.h glob-cpp/glob.h glob-cpp/token.def)
target_include_directories(oshi PUBLIC ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR}) # binary dir contains zconf.h
target_link_libraries(oshi pugixml zlib)

if(WIN32) # on Windows copy dlls to output directory
cmake_minimum_required(VERSION 3.21)
add_custom_command(TARGET oshi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:oshi> $<TARGET_FILE_DIR:oshi>
        COMMAND_EXPAND_LISTS)
endif()

# file(ARCHIVE_EXTRACT INPUT ${jmdict_SOURCE_DIR}/JMdict_e.gz) # does not work, CMake does not support gzip (just tar+gzip)
file(COPY ${jmdict_SOURCE_DIR}/JMdict_e.gz
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/grammar.rules
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
