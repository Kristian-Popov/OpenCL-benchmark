# Build instructions for Windows Debug x64 version

Run commands in MSBuild Command Prompt for VS 2015.

1. Download and build Boost 1.65.1

```bash
    Invoke-WebRequest -Uri "https://dl.bintray.com/boostorg/release/1.65.1/source/boost_1_65_1.7z" -OutFile "boost_1_65_1.7z"
```

Note: if you don't have 7Zip installed, change extension to zip, but zip version is twice as large.
Unpack using 7Zip or other archive program to folder boost_1_65_1.
Number of simoultanous jobs is defined by -j parameter, feel free to select a value appropriate for your processor (usually number of threads).

```bash
    cd boost_1_65_1/tools/build
    bootstrap.bat
    b2 --build-dir=build/x64 address-model=64 threading=multi --build-type=complete --stagedir=./stage/x64 --toolset=msvc-14.0 -j 8
```

It will take quite some time, so go get a cup of coffee and do something else.

More information can be found on [this blog post](http://informilabs.com/building-boost-32-bit-and-64-bit-libraries-on-windows/) , thank you Justin!
Link to [Boost Getting started page for Windows](https://www.boost.org/doc/libs/1_65_1/more/getting_started/windows.html)
2. Download library code

```bash
    git clone https://github.com/Kristian-Popov/OpenCL-benchmark.git
    cd OpenCL-benchmark
    git submodule update --init --recursive
```

**Steps below are needed only if you want to build examples and unit tests**
3. Generate Visual Studio solution

```bash
    mkdir build
    cd build
    cmake -G "Visual Studio 14 2015 Win64" -D BOOST_ROOT:PATH=path/to/boost/root -D KPV_CL_BENCH_BUILD_EXAMPLES=ON -D KPV_CL_BENCH_BUILD_TESTS=ON -D OPENCL_ICD_LOADER_REQUIRE_WDK=OFF ..
```

4. Finally build the executables

```bash
    msbuild kpv_cl_benchmark.sln
```

Examples executable can be found at

```bash
    cd examples\Debug
    kpv_cl_benchmark_examples.exe
```

Results file can be found in the same folder, default file name is output.json

Unit test executable can be found at

```bash
    cd tests\Debug
    kpv_cl_benchmark_tests.exe
```

For detailed description of command line parameters of unit test executable, please consult Catch2 documentation.