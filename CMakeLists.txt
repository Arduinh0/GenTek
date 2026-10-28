cmake_minimum_required(VERSION 3.10)
project(tsp_ga LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Modo Release por padrão para medição de telemetria em alta performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Flags de otimização máxima para C++ moderno
if(MSVC)
    add_compile_options(/O2 /Oi /Ot /permissive-)
else()
    add_compile_options(-O3 -march=native)
endif()

# Incluir headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Coletar sources
set(SOURCES
    src/distance.cpp
    src/fitness.cpp
    src/generators.cpp
    src/genetic.cpp
    main.cpp
)

add_executable(tsp_ga ${SOURCES})
