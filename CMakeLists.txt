cmake_minimum_required(VERSION 3.19)
project(SerializationDoublyLinkList)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(serialization main.cpp)

# Создаём тестовый входной файл inlet.in (пример из ТЗ)
file(WRITE ./inlet.in "apple;2\nbanana;-1\ncarrot;1\n")