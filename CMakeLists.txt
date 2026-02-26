cmake_minimum_required(VERSION 3.19)
project(SerializationDoublyLinkList LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(serialization main.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(serialization PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(serialization PRIVATE /W4)
endif()

# Создаём тестовый входной файл inlet.in (пример из ТЗ)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/inlet.in" "apple;2\nbanana;-1\ncarrot;1\n")

# Цель для быстрого запуска
add_custom_target(run
    COMMAND serialization
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS serialization
    COMMENT "Running serialization..."
)