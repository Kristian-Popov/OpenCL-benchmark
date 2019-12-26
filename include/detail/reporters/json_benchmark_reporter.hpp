#ifndef KPV_REPORTERS_JSON_BENCHMARK_REPORTER_H_
#define KPV_REPORTERS_JSON_BENCHMARK_REPORTER_H_

#include "devices/platform_list.hpp"
#include "indicators/duration_indicator.hpp"

namespace kpv {
namespace cl_benchmark {
class JsonBenchmarkReporter {
public:
    JsonBenchmarkReporter(const std::string& file_name) : file_name_(file_name) {}

    void Initialize(const PlatformList& platform_list) {
        tree_["baseInfo"] = {
            {"about", "This file was built by OpenCL benchmark."},
            {"time", GetCurrentTimeString()},
            {"formatVersion", "0.1.0"}};
        for (auto& platform : platform_list.AllPlatforms()) {
            nlohmann::json devices = nlohmann::json::array();
            for (auto& device : platform->GetDevices()) {
                devices.push_back(device->UniqueName());
            }
            tree_["deviceList"][platform->Name()] = devices;
        }
        tree_["fixtureFamilies"] = nlohmann::json::array();
    }

    void AddFixtureFamilyResults(const FixtureFamilyResult& results) {
        using nlohmann::json;

        json fixture_family_tree = {{"name", results.name}};
        if (results.element_count) {
            fixture_family_tree["elementCount"] = results.element_count.value();
        }

        // Add array of step names to preserve their order
        std::vector<std::string> step_names(results.steps.size());
        for (const auto& v : results.steps) {
            step_names[v.second.order] = v.first;
        }
        fixture_family_tree["steps"] = step_names;

        json fixture_tree = json::array();
        for (auto& data : results.benchmark) {
            // Add fixture name
            json current_fixture_tree = json::object({{"name", data.first.Serialize()}});

            // Add number of iterations, if any
            // Otherwise add failure reason
            size_t iteration_count = data.second.iterations.size();
            if (iteration_count > 0) {
                current_fixture_tree["iterationCount"] = iteration_count;
                DurationIndicator indicator{data.second};
                indicator.SerializeValue(current_fixture_tree);
            } else if (data.second.failure_reason) {
                current_fixture_tree["failureReason"] = data.second.failure_reason.value();
            }

            fixture_tree.push_back(current_fixture_tree);
        }

        fixture_family_tree["fixtures"] = fixture_tree;
        tree_["fixtureFamilies"].push_back(fixture_family_tree);
    }

    /*
    Optional method to flush all contents to output
    */
    void Flush() {
        try {
            std::ofstream o(file_name_);
            o.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
            if (pretty_) {
                o << std::setw(4);
            }
            o << tree_ << std::endl;
        } catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error)
                << "Caught exception when building report in JSON format and writing to a file "
                << file_name_ << ": " << e.what();
            throw;
        }
    }

private:
    // TODO move to some free function?
    std::string GetCurrentTimeString() {
        // TODO replace with some library?
        // Based on https://stackoverflow.com/a/10467633
        time_t now = time(nullptr);
        struct tm tstruct;
        char buf[80];
        // TODO gmtime is not thread-safe, do something with that?
        tstruct = *gmtime(&now);
        strftime(buf, sizeof(buf), "%FT%TZ", &tstruct);
        return buf;
    }

    static const bool pretty_ = true;  // TODO make configurable?
    std::string file_name_;
    nlohmann::json tree_;
};

}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_REPORTERS_JSON_BENCHMARK_REPORTER_H_
