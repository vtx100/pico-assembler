cmake_minimum_required(VERSION 3.15)
project(pico-assembler LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
    src/status.c
    src/hashmap.c
    src/token_list.c
    src/io.c
    src/lexer.c
      src/parser.c    
     src/linker.c    
    src/main.c    
)

add_compile_options(
     $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
