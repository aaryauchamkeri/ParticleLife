cmake_minimum_required(VERSION 3.10)

project(ParticleLife VERSION 1.0)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(ParticleLife 
    src/main.cpp
    src/ThreadPool.cpp
)

target_include_directories(ParticleLife PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ParticleLife ${SDL2_LIBRARIES})