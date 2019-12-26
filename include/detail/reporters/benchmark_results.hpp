#ifndef KPV_REPORTERS_BENCHMARK_RESULTS_H_
#define KPV_REPORTERS_BENCHMARK_RESULTS_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "boost/optional.hpp"
#include "duration.hpp"
#include "fixtures/fixture_family.hpp"
#include "fixtures/fixture_id.hpp"

namespace kpv {
namespace cl_benchmark {

struct IterationInfo {
    std::unordered_map<std::string /* step name */, Duration> durations;
};

struct FixtureResult {
    std::vector<IterationInfo> iterations;

    boost::optional<std::string> failure_reason;
};

struct StepInfo {
    int order = 0;  // Order in which this step was added

    StepInfo(int _order) : order(_order) {}
};

struct FixtureFamilyResult {
    std::unordered_map<FixtureId, FixtureResult> benchmark;
    std::unordered_map<std::string /* step name */, StepInfo> steps;
    std::string name;
    boost::optional<int32_t> element_count;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_REPORTERS_BENCHMARK_RESULTS_H_
