cmake_minimum_required(VERSION 3.30)
project(hashTable LANGUAGES CXX) # TODO add ASM

# TODO target_compile_features() for standard setting
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON) # TODO find out
set(CMAKE_CXX_EXTENSIONS OFF)

# TODO RELEASE_FLAGS and target_compile_options(...) + target_compile_definitions(...)
SET(CMAKE_CXX_FLAGS "-g -D NDEBUG -D OPTIMIZE_CRC32 -D OPTIMIZE_STRCMP -D OPTIMIZE_CREATE_NODE -D OPTIMIZE_ADLER32 -O3 -mavx2 -mavx -march=native -msse4.2 -no-pie")
add_executable(hashTable
    main.cpp
        src/hashFunctions.cpp
        src/IO.cpp
        customWarning/colorPrint/colorPrint.cpp
)
# TODO separate lib for colorPrint

target_sources(hashTable PRIVATE createNode_asm/createNode.o)

# TODO
# add_library(list-lib INTERFACE)
# target_include_directories(list-lib PUBLIC list/)
# target_link_library(...)

# TODO use list(TRANSFORM ... PREPEND ...)
target_include_directories(hashTable PRIVATE
    list/
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/customWarning/colorPrint
    ${CMAKE_CURRENT_SOURCE_DIR}/customWarning/
)
