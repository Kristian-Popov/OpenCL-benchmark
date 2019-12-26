#ifndef KPV_INDICATORS_DURATION_INDICATOR_H_
#define KPV_INDICATORS_DURATION_INDICATOR_H_

#include <boost/optional.hpp>
#include <unordered_map>

#include "duration.hpp"
#include "indicator_interface.hpp"
#include "reporters/benchmark_results.hpp"

namespace kpv {
namespace cl_benchmark {
class DurationIndicator : public IndicatorInterface {
public:
    explicit DurationIndicator(const FixtureResult& benchmark) { Calculate(benchmark); }

    void SerializeValue(nlohmann::json& tree) override {
        if (calculated_.iteration_count == 1) {
            // We have a single duration
            // TODO change to store full results instead of this trickery?
            for (auto& step_data : calculated_.step_durations) {
                tree["fullDuration"][step_data.first] = nlohmann::json::array({step_data.second});
            }
        } else if (calculated_.iteration_count > 1) {
            // We have a duration range
            for (auto& step_data : calculated_.step_durations) {
                tree["compressedDuration"][step_data.first]["avg"] = step_data.second;
                tree["compressedDuration"][step_data.first]["min"] =
                    calculated_.step_min_durations.at(step_data.first);
                tree["compressedDuration"][step_data.first]["max"] =
                    calculated_.step_max_durations.at(step_data.first);
            }
        }
    }

private:
    struct FixtureCalculatedData {
        std::unordered_map<std::string, Duration> step_durations;
        std::unordered_map<std::string, Duration> step_min_durations;
        std::unordered_map<std::string, Duration> step_max_durations;
        std::size_t iteration_count;
        // Is not serialized, is just a temporary solution to check if duration is empty
        // TODO check in some better way?
        Duration total_duration;
    };

    void Calculate(const FixtureResult& benchmark) {
        if (!benchmark.iterations.empty()) {
            // Calculate duration for every step and total one.
            Duration total_duration;

            for (auto& iter_results : benchmark.iterations) {
                for (auto& step_results : iter_results.durations) {
                    calculated_.step_durations[step_results.first] += step_results.second;
                    total_duration += step_results.second;
                    {
                        auto iter = calculated_.step_min_durations.find(step_results.first);
                        if (iter == calculated_.step_min_durations.end()) {
                            iter = calculated_.step_min_durations
                                       .emplace(step_results.first, Duration::Max())
                                       .first;
                        }
                        if (step_results.second < iter->second) {
                            iter->second = step_results.second;
                        }
                    }
                    {
                        auto iter = calculated_.step_max_durations.find(step_results.first);
                        if (iter == calculated_.step_max_durations.end()) {
                            iter = calculated_.step_max_durations
                                       .emplace(step_results.first, Duration::Min())
                                       .first;
                        }
                        if (step_results.second > iter->second) {
                            iter->second = step_results.second;
                        }
                    }
                }
            }

            // Divide durations by amount of iterations
            calculated_.iteration_count = benchmark.iterations.size();
            total_duration /= calculated_.iteration_count;
            for (auto& p : calculated_.step_durations) {
                p.second /= calculated_.iteration_count;
            }

            calculated_.total_duration = total_duration;
        }
    }

    FixtureCalculatedData calculated_;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_INDICATORS_DURATION_INDICATOR_H_
