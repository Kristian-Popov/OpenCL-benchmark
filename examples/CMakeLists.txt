# CMake project for examples executable

project(kpv_cl_benchmark_examples LANGUAGES CXX)

add_executable (${PROJECT_NAME} 
    examples-main.cpp
    fixtures/factorial_opencl_fixture.cpp
    fixtures/factorial_opencl_fixture.h
    fixtures/cuboid_opencl_fixture.cpp
    fixtures/cuboid_opencl_fixture.h
)

# target_include_directories (${PROJECT_NAME}  PUBLIC ${OpenCL_INCLUDE_DIRS} 
# 	${CMAKE_SOURCE_DIR}/contrib 
	# ${Boost_INCLUDE_DIRS} 
# )
target_link_libraries (${PROJECT_NAME} kpv_cl_benchmark)

if (UNIX)
    # target_link_libraries (${PROJECT_NAME}  pthread)
endif (UNIX)

set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_STANDARD_REQUIRED ON)

# TODO enable warnings