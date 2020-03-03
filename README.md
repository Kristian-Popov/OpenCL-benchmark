# OpenCL/C++ microbenchmark library

OpenCL benchmark framework that simplifies benchmarking of OpenCL kernel code with C++ host code.
This library is specifically targeted for cases when comparison between different algorithms, implementations is needed, same as performance comparison on heterogeneous OpenCL devices.
This is accomplished by running OpenCL fixtures on all OpenCL devices found in a system. Additionally the following features are implemented:

1. Performance comparison of different algorithms
2. Automatic or manual selection of iteration count
3. Fixtures that need initialization and finalization are supported
4. Seamless Boost Compute integration
5. Statistics - minimum, maximum and average run time (more to come)
6. Multi-step fixtures
7. Some additional information based on run time - number of elements processed per second, processing time for a single element (more to come)
8. Pick number of iterations automatically or pick/limit them manually

Library consists of two parts:

1. C++ library that facilitates writing and executing test fixtures. Library is header-only, which means simplified build process. CMake file is provided to simplify linking to Boost and other dependencies and remove dependency on OpenCL SDK.
2. Viewer application, implemented in TypeScript.

## Usage

First and foremost, some terminology:

* Step - performs one particular operation, like memory transfer or kernel execution, corresponds to a single OpenCL event
Represented as one cell in a table in viewer app.
* Fixture - performs one particular kind of test, for one OpenCL device.
Consists of one or multiple steps. Represented as one row in a table.
* Fixture family - contains all fixtures that perform one test but on different devices or using different algorithms (same calculation result and identical algorithm parameters).
All fixtures in a one family are compared with each other and represented as one table.
* Fixture category - logically connects fixture families that execute similar calculations

To test performance of some code, a fixture should be implemented - it must derive from [kpv::cl_benchmark::Fixture class](include/detail/fixtures/fixture.hpp).
After that a function that builds a fixture family has to be created. Fixture family has some additional information like name, fixture list, optional element count.
This function should be registered by macro [REGISTER_FIXTURE](include/detail/fixture_register_macros.hpp). You can also use std::bind to pass additional parameters to this function. Complete example can be found at [example.cpp](examples/examples-main.cpp).

Library has ready implementation of function main(), that is included with header [cl_benchmark_main.hpp](include/cl_benchmark_main.hpp). This macro has to be defined exactly once in one implementation .cpp file.
Generated executable has the following command line options:

* -i X, --iterations X: set number of iterations exactly to X (disables automatic selection), mutually exclusive with -m and -M
* -m X, --min-iterations X: limit minimum number of iterations to X
* -M X, --max-iterations X: limit maximum number of iterations to X
* t X, --target-time X: target execution time of one fixture (examples: 100ms, 1.5ns, 9s). Default value is 100ms
* --additional-params params: additional parameters that are passed to fixtures
* -c, --cpu: run fixtures on OpenCL CPU devices
* -g, --gpu: run fixtures on OpenCL GPU devices
* --other-devices: run fixtures on OpenCL accelerators and other devices

Due to complexity of output data, the only supported output method is JSON file. Standard output is used for logging.

Viewer's purpose is to visually show results in convenient form. Viewer is a simple web page implemented in TypeScript. Easy way to deploy local web server with a viewer app is to use parcel.

```bash
    npm install
    parcel index.html
```

Open your browser and [open this link](http://localhost:1234) and you're ready to go!
To see prepared results, load output.json file to upload field on a website.

Viewer is still quite raw, so you'll have click on any table header to open the table.

## Build

To build test executables, you need the following libraries:

1. Boost 1.65.1 (Compute, Program options, log and others)
2. JSON for Modern C++ (*)

Additional dependencies:

1. Catch2 (needed to build library's tests) (*)
2. Khronos OpenCL ICD Loader with OpenCL Headers (needed if you don't have OpenCL SDK installed) (*)

Libraries with (*) are downloaded as submodules and are part of CMake build process, so you don't have to worry about them.

Note: OpenCL SDK is not required for building, but any OpenCL implementation (i.e. OpenCL.dll library) is needed to run the executable.

Library is implemented in C++14, so sufficiently modern compiler is required. Tested with:

1. Windows 10, Visual Studio 2015
2. Ubuntu, g++ 7.4, clang 9.0.0

Software needed to use this library:

* CMake (build system)
* Compiler/IDE (Visual Studio 2015 for Windows, g++/clang++ for Linux)
* OpenCL implementation (e.g. POCL for Linux)

Build instructions for [Windows 10, Debug x64 version](BUILD_WINDOWS.md), [Ubuntu 18.04 Bionic Beaver, Debug x64 version](BUILD_UBUNTU.md).

## Versioning

This library uses [semantic versioning](https://semver.org/), current version is 0.1.0, API is considered unstable, incomplete and anything may change at any time.

## Repository structure

* [include/](include) - library headers
* [examples/](examples) - fixture examples
* [tests/](tests) - unit tests
* [contrib/](contrib) - third-party dependencies

## Planned improvements

1. More statistical values
2. Remove strict dependency on Boost Compute
3. Support for host processor fixtures so implementations running on a host processor can be compared with OpenCL devices

## Contributing and License

Contributors are welcome!
C++ part of this project generally follows Google C++ code style with changes: [code convention](code_convention.md).
Please use supplied clang format style file.
