cmake_minimum_required(VERSION 3.10)
project(tsp_ga LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
