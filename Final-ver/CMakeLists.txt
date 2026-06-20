cmake_minimum_required(VERSION 3.10)

project(file_zipper)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Huffman library
add_library(
    huffman
    STATIC
    src/Huffman.cpp
    src/BitWriter.cpp
    src/BitReader.cpp
)

# LZ77 library
add_library(
    LZ77
    STATIC
    src/LZ77.cpp
)

# Main executable
add_executable(
    zipper
    src/main.cpp
)

target_link_libraries(
    zipper
    huffman
    LZ77
    TokenIO
)

# Huffman test
add_executable(
    test_huffman
    tests/test_huffman.cpp
)

target_link_libraries(
    test_huffman
    huffman
)

add_test(
    NAME huffman_test
    COMMAND test_huffman
)

# LZ77 test
add_executable(
    test_LZ77
    tests/test_LZ77.cpp
)

target_link_libraries(
    test_LZ77
    LZ77
)

add_test(
    NAME LZ77_test
    COMMAND test_LZ77
)
add_library(
    TokenIO
    STATIC
    src/TokenIO.cpp
)
add_executable(
    test_TokenIO
    tests/test_TokenIO.cpp
)
target_link_libraries(
    test_TokenIO
    TokenIO
    LZ77
)
add_test(
    NAME test_TokenIO
    COMMAND test_TokenIO
)

add_executable(
    test_token_stream
    tests/test_token_stream.cpp
)
target_link_libraries(
    test_token_stream
    TokenIO
    LZ77
    huffman
)
add_test(
    NAME test_token_stream
    COMMAND test_token_stream
)
add_executable(
    test_symbol_stream
    tests/test_symbol_stream.cpp
)
target_link_libraries(
    test_symbol_stream
    TokenIO
    LZ77
    huffman
)
add_test(
    NAME test_symbol_stream
    COMMAND test_symbol_stream
)
