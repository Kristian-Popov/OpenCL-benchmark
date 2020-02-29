# Main CMake project for OpenCL benchmark library

# TODO is this version correct?
cmake_minimum_required (VERSION 3.1)

project(kpv_cl_benchmark LANGUAGES CXX)

option(KPV_CL_BENCH_BUILD_EXAMPLES "Build the examples." OFF)
option(KPV_CL_BENCH_BUILD_TESTS "Build the tests." OFF)
option(KPV_CL_NO_OPENCL "Do not use OpenCL. Can be turned off only if you don't need to run OpenCL fixtures." OFF)

# Boost should be located in default OS location (often a case on Linux),
# otherwise set BOOST_ROOT variable to path of Boost distribution
# Minimal Boost version is 1.65.1
find_package(Boost 1.65.1 REQUIRED COMPONENTS log program_options)

# Look for libraries in the following folders
link_directories(${Boost_LIBRARY_DIRS})

# Add nlohmann_json library and disable its unit tests
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_MultipleHeaders OFF CACHE INTERNAL "")
add_subdirectory( contrib/nlohmann-json )

add_library( ${PROJECT_NAME} INTERFACE )
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)

if(NOT KPV_CL_NO_OPENCL)
    # Find OpenCL and Boost libraries.
    find_package(OpenCL)
    # If OpenCL was not found and we need it, use headers in contrib folder and ICD Loader library By Khronos
    # (OpenCL.lib)
    # Using this hack removes the necessity of OpenCL SDK on Windows
    # TODO info about Linux shared objects
    # TODO replace with clew?
    if(WIN32 AND (NOT OpenCL_FOUND))
        message( "OpenCL SDK not found, build ICD Loader instead" )
        add_definitions( -DCL_TARGET_OPENCL_VERSION=220 )
        set(OPENCL_ICD_LOADER_REQUIRE_WDK OFF) # Do not require Windows Driver Kit
        file( COPY ${CMAKE_SOURCE_DIR}/contrib/OpenCL-Headers/CL
            DESTINATION ${CMAKE_SOURCE_DIR}/contrib/Khronos-OpenCL-ICD-Loader/inc )
        add_subdirectory( ${CMAKE_SOURCE_DIR}/contrib/Khronos-OpenCL-ICD-Loader)
        set(OpenCL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/contrib/OpenCL-Headers/)
        set(OpenCL_LIBRARY ${CMAKE_SOURCE_DIR}/contrib/Khronos-OpenCL-ICD-Loader/)
        set(OpenCL_LIBRARIES OpenCL)
    endif()
endif()

target_include_directories( ${PROJECT_NAME} INTERFACE
    include
    ${Boost_INCLUDE_DIRS}
    ${OpenCL_INCLUDE_DIRS}
)
target_link_libraries( ${PROJECT_NAME} INTERFACE
    ${Boost_LIBRARIES}
    ${OpenCL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

if(KPV_CL_BENCH_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(KPV_CL_BENCH_BUILD_TESTS)
    # CTest is not used at this point
    # enable_testing()
    add_subdirectory(tests)
endif()
