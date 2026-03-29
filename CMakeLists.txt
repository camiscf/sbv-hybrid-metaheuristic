cmake_minimum_required(VERSION 3.10)
project(sbv_otimizacao)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

add_executable(sbv
    src/main.cpp
    src/instancia.cpp
    src/solucao.cpp
    src/busca_local.cpp
)