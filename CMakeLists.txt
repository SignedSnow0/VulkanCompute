cmake_minimum_required(VERSION 3.16)
project(RayTracingSequenziale)

find_package(OpenMP REQUIRED)

add_executable(RayTracingSequenziale
    main.cpp
    mesh.cpp
    math.cpp
    vec.hpp
    vec.cpp
)

target_link_libraries(RayTracingSequenziale PRIVATE OpenMP::OpenMP_CXX)