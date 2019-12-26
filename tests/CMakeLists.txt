# CMake project for tests executable

project(kpv_cl_benchmark_tests LANGUAGES CXX)

add_executable (${PROJECT_NAME} 
    tests.cpp
    duration_tests.cpp
)

target_include_directories (${PROJECT_NAME}  PUBLIC
    ${CMAKE_SOURCE_DIR}/contrib/catch/single_include/catch2
)
target_link_libraries (${PROJECT_NAME} kpv_cl_benchmark)

if (UNIX)
    # target_link_libraries (${PROJECT_NAME}  pthread)
endif (UNIX)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD_REQUIRED ON)

# TODO enable warnings