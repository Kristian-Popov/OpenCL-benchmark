# Build instructions for Ubuntu 18.04 Bionic Beaver, Debug x64 version

Run commands in terminal.

1. Download and install Boost, POCL, ICD, OpenCL headers, make

```bash
    sudo apt-get install libboost-log1.65-dev libboost-program-options1.65-dev libpocl-dev opencl-headers make
```

3. Download and install compiler

For GCC

```bash
    sudo apt-get install g++
```

For clang

```bash
    sudo apt-get install clang-9
```

2. Download library code

```bash
    git clone https://github.com/Kristian-Popov/OpenCL-benchmark.git
    cd OpenCL-benchmark
    git submodule update --init --recursive
```

**Steps below are needed only if you want to build examples and unit tests**
3. Generate Makefile

For default system compiler:

```bash
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -D KPV_CL_BENCH_BUILD_EXAMPLES=ON -D KPV_CL_BENCH_BUILD_TESTS=ON -D Boost_USE_STATIC_LIBS=ON ..
```

For clang:

```bash
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -D KPV_CL_BENCH_BUILD_EXAMPLES=ON -D KPV_CL_BENCH_BUILD_TESTS=ON -D Boost_USE_STATIC_LIBS=ON -D CMAKE_CXX_COMPILER=clang++-9 ..
```

4. Finally build the executables

```bash
    make
```

Examples executable can be found at

```bash
    cd examples/Debug
    ./kpv_cl_benchmark_examples
```

Results file can be found in the same folder, default file name is output.json.

Unit test executable can be found at

```bash
    cd tests/Debug
    ./kpv_cl_benchmark_tests
```

For detailed description of command line parameters of unit test executable, please consult Catch2 documentation.