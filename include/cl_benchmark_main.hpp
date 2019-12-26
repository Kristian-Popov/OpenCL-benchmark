#ifndef KPV_CL_BENCHMARK_MAIN_H_
#define KPV_CL_BENCHMARK_MAIN_H_

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cstdlib>

#include "cl_benchmark.hpp"

int main(int argc, char** argv) {
    try {
        using namespace kpv::cl_benchmark;
        CommandLineProcessor processor;
        RunSettings settings;
        if (processor.Process(argc, argv, settings)) {
            FixtureRunner fixture_runner;
            fixture_runner.Run(settings);
        }

    } catch (std::exception& e) {
        BOOST_LOG_TRIVIAL(fatal) << "Caught fatal exception: " << e.what();
        throw;
    } catch (...) {
        BOOST_LOG_TRIVIAL(fatal) << "Caught fatal error";
        throw;
    }

    return EXIT_SUCCESS;
}

#endif  // KPV_CL_BENCHMARK_MAIN_H_
