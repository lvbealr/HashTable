cmake_minimum_required(VERSION 3.30)
project(hashTable LANGUAGES CXX ASM_NASM)

# colorPrint static library
add_library(colorPrint STATIC
    customWarning/colorPrint/colorPrint.cpp
)
target_include_directories(colorPrint PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/customWarning/colorPrint
    ${CMAKE_CURRENT_SOURCE_DIR}/customWarning
)

# linkedList static library
add_library(linkedList INTERFACE)
target_include_directories(linkedList INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/list)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/createNode.o
    COMMAND nasm -f elf64 -g ${CMAKE_CURRENT_SOURCE_DIR}/createNode_asm/createNode.asm -o ${CMAKE_CURRENT_BINARY_DIR}/createNode.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/createNode_asm/createNode.asm
    COMMENT "Compiling createNode.asm with NASM"
)

set(SOURCES
    main.cpp
    src/hashFunctions.cpp
    src/IO.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/createNode.o
)

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

function(add_hash_table_target TARGET_NAME OPTIMIZATION_LEVEL DEFINITIONS COMPILER_OPTIONS)
    add_executable(${TARGET_NAME} ${SOURCES})
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
    target_include_directories(${TARGET_NAME} PRIVATE ${INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE colorPrint linkedList)
    target_compile_definitions(${TARGET_NAME} PRIVATE NDEBUG ${DEFINITIONS})
    target_compile_options(${TARGET_NAME} PRIVATE ${OPTIMIZATION_LEVEL} ${COMPILER_OPTIONS})
    target_link_options(${TARGET_NAME} PRIVATE -no-pie)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${TARGET_NAME} PRIVATE -g)
        target_compile_definitions(${TARGET_NAME} PRIVATE -U NDEBUG)
    endif()
endfunction()

# -O0
add_hash_table_target(hashTable_default_default "" "" "-g")

# -O3
add_hash_table_target(hashTable_default_o3 "-O3" "" "-g")

# OPTIMIZE_CRC32 
add_hash_table_target(hashTable_optimize_crc32 "-O3" "OPTIMIZE_CRC32" "-mavx2;-mavx;-msse4.2;-no-pie;-g")

# OPTIMIZE_CRC32 & OPTIMIZE_STRCMP
add_hash_table_target(hashTable_optimize_crc32_strcmp "-O3" "OPTIMIZE_CRC32;OPTIMIZE_STRCMP" "-mavx;-mavx2;-msse4.2;-no-pie;-g")

# OPTIMIZE_CRC32 & OPTIMIZE_STRCMP & OPTIMIZE_CREATE_NODE
add_hash_table_target(hashTable_optimize_crc32_strcmp_create_node "-O3" "OPTIMIZE_CRC32;OPTIMIZE_STRCMP;OPTIMIZE_CREATE_NODE" "-mavx;-mavx2;-msse4.2;-no-pie;-g")

# OPTIMIZE_CRC32 & OPTIMIZE_STRCMP & OPTIMIZE_CREATE_NODE & OPTIMIZE_ADLER32
add_hash_table_target(hashTable_optimize_crc32_strcmp_create_node_adler32 "-O3" "OPTIMIZE_CRC32;OPTIMIZE_STRCMP;OPTIMIZE_CREATE_NODE;OPTIMIZE_ADLER32" "-mavx;-mavx2;-msse4.2;-no-pie;-g")