#ifndef KPV_FIXTURE_RUNNER_H_
#define KPV_FIXTURE_RUNNER_H_

#include <algorithm>
#include <boost/algorithm/clamp.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "detail/devices/opencl_device.hpp"
#include "detail/devices/platform_list.hpp"
#include "detail/duration.hpp"
#include "detail/fixture_registry.hpp"
#include "detail/fixtures/fixture.hpp"
#include "detail/fixtures/fixture_family.hpp"
#include "detail/reporters/json_benchmark_reporter.hpp"
#include "detail/run_settings.hpp"

namespace kpv {
namespace cl_benchmark {
// TODO forbid construction outside allowed context if possible
class FixtureRunner {
public:
    void Run(RunSettings settings) {  // TODO force Run to be executable one time only?
        BOOST_LOG_TRIVIAL(info) << "Welcome to OpenCL benchmark.";

        if (!((settings.min_iterations >= 1) && (settings.max_iterations >= 1))) {
            throw std::invalid_argument(
                "Minimum or maximum number of iterations is incorrect (less than 1).");
        }

        std::shared_ptr<FixtureRegistry> fixture_registry = FixtureRegistry::instance().lock();
        if (!fixture_registry) {
            throw std::runtime_error("Fixture registry was not constructed.");
        }
        std::vector<std::string> present_categories = fixture_registry->GetAllCategories();
        std::sort(present_categories.begin(), present_categories.end());
        std::sort(settings.category_list.begin(), settings.category_list.end());

        std::unordered_set<std::string> categories_to_run;

        // Build a list of categories that are mentioned in command line but don't exist
        std::vector<std::string> missing_categories;
        std::set_difference(
            settings.category_list.cbegin(), settings.category_list.cend(),
            present_categories.cbegin(), present_categories.cend(),
            std::back_inserter(missing_categories));
        if (!missing_categories.empty()) {
            BOOST_LOG_TRIVIAL(warning)
                << "Fixture categories \"" << VectorToString(missing_categories)
                << "\" are in include/exclude list but do not exist.";
        }

        // Build a list of categories to run
        if (settings.operation == RunSettings::kRunAllExcept) {
            std::set_difference(
                present_categories.cbegin(), present_categories.cend(),
                settings.category_list.cbegin(), settings.category_list.cend(),
                std::inserter(categories_to_run, categories_to_run.begin()));
        } else if (settings.operation == RunSettings::kRunOnly) {
            std::set_intersection(
                present_categories.cbegin(), present_categories.cend(),
                settings.category_list.cbegin(), settings.category_list.cend(),
                std::inserter(categories_to_run, categories_to_run.begin()));
        } else if (settings.operation == RunSettings::kList) {
            // List all categories
            BOOST_LOG_TRIVIAL(info) << "List of categories" << std::endl
                                    << VectorToString(present_categories);
            return;
        } else {
            BOOST_LOG_TRIVIAL(error) << "Selected fixture filter method is not supported. Exiting.";
            return;
        }

        JsonBenchmarkReporter reporter(settings.output_file_name);
        PlatformList platform_list(settings.device_config);
        reporter.Initialize(platform_list);

        BOOST_LOG_TRIVIAL(info) << "We have " << categories_to_run.size()
                                << " fixture categories to run";

        int family_index = 1;  // Used for logging only
        for (auto& p : *fixture_registry) {
            if (categories_to_run.count(p.first) == 0) {
                // This fixture is in exclude list, skip it
                BOOST_LOG_TRIVIAL(info) << "Skipping category " << p.first;
                continue;
            }

            FixtureFamily fixture_family = p.second(platform_list);
            std::string fixture_name = fixture_family.name;
            FixtureFamilyResult ff_result;  // Short for "fixture family result"
            ff_result.name = fixture_family.name;
            ff_result.element_count = fixture_family.element_count;

            BOOST_LOG_TRIVIAL(info) << "Starting fixture family \"" << fixture_name << "\"";

            for (auto& fixture_data : fixture_family.fixtures) {
                const FixtureId& fixture_id = fixture_data.first;
                std::shared_ptr<Fixture>& fixture = fixture_data.second;
                FixtureResult fixture_result;

                BOOST_LOG_TRIVIAL(info)
                    << "Starting run on device \"" << fixture_id.device()->Name() << "\"";

                try {
                    std::vector<std::string> required_extensions = fixture->GetRequiredExtensions();
                    std::sort(required_extensions.begin(), required_extensions.end());

                    std::vector<std::string> have_extensions = fixture_id.device()->Extensions();
                    std::sort(have_extensions.begin(), have_extensions.end());

                    std::vector<std::string> missed_extensions;
                    std::set_difference(
                        required_extensions.cbegin(), required_extensions.cend(),
                        have_extensions.cbegin(), have_extensions.cend(),
                        std::back_inserter(missed_extensions));
                    if (!missed_extensions.empty()) {
                        fixture_result.failure_reason = "Required extension(s) are not available";
                        // Destroy fixture to release some memory sooner
                        fixture.reset();

                        BOOST_LOG_TRIVIAL(warning)
                            << "Device \"" << fixture_id.device()->Name()
                            << "\" doesn't support extensions needed for fixture: "
                            << VectorToString(missed_extensions);

                        ff_result.benchmark.insert(std::make_pair(fixture_id, fixture_result));

                        continue;
                    }

                    fixture->Initialize();  // TODO move higher when fixture is constructed, may be
                                            // disable altogether?

                    // Warm-up for one iteration to get estimation of execution time
                    RuntimeParams params;
                    params.additional_params = settings.additional_params;

                    EventList warmup_ev_list = fixture->Execute(params);
                    IterationInfo warmup_result =
                        AddIteration(warmup_ev_list, ff_result, fixture_result);

                    Duration total_operation_duration = std::accumulate(
                        warmup_result.durations.cbegin(), warmup_result.durations.cend(),
                        Duration(), [](Duration val, const std::pair<std::string, Duration>& p) {
                            return val + p.second;
                        });
                    int iteration_count =
                        boost::algorithm::clamp<int>(
                            settings.target_execution_time / total_operation_duration,
                            settings.min_iterations, settings.max_iterations) -
                        1;
                    if (!(iteration_count >= 0)) {
                        throw std::logic_error(
                            "Estimated number of iterations is incorrect (less than 0).");
                    }

                    if (settings.verify_results) {
                        fixture->VerifyResults();
                    }

                    if (settings.store_results) {
                        fixture->StoreResults();
                    }

                    for (int i = 0; i < iteration_count; ++i) {
                        EventList ev_list = fixture->Execute(params);
                        AddIteration(ev_list, ff_result, fixture_result);
                    }

                    fixture->Finalize();
                } catch (boost::compute::opencl_error& e) {
                    BOOST_LOG_TRIVIAL(error) << "OpenCL error occured: " << e.what();
                    fixture_result.failure_reason = e.what();
                } catch (std::exception& e) {
                    BOOST_LOG_TRIVIAL(error) << "Exception occured: " << e.what();
                    fixture_result.failure_reason = e.what();
                }

                // Destroy fixture to release some memory sooner
                fixture.reset();

                BOOST_LOG_TRIVIAL(info)
                    << "Finished run on device \"" << fixture_id.device()->Name() << "\"";

                ff_result.benchmark.insert(std::make_pair(fixture_id, fixture_result));
            }

            reporter.AddFixtureFamilyResults(ff_result);

            BOOST_LOG_TRIVIAL(info)
                << "Fixture family \"" << fixture_name << "\" finished successfully.";
            ++family_index;
        }
        reporter.Flush();

        BOOST_LOG_TRIVIAL(info) << "Done";
    }

private:
    template <typename T>
    std::string VectorToString(const std::vector<T>& v, const std::string& delimiter = ", ") {
        std::stringstream result;
        auto iter1 = v.cbegin();
        auto iter2 = iter1;
        ++iter2;
        for (; iter1 != v.cend(); ++iter1) {
            result << *iter1;
            if (iter2 != v.cend()) {
                result << delimiter;
                ++iter2;
            }
        }
        // It could be easily implemented as
        // std::copy(v.begin(), v.end(), std::ostream_joiner<T>(result, delimiter.c_str()));
        // But this requires ostream_joiner which is at a moment of writing in std::experimental
        return result.str();
    }

    void WaitForEventList(EventList& event_list) {
        // Wait for events in reverse order.
        // In fact waiting for the last one is sufficient for in-order queues,
        // but waiting on all of them covers the case of out-of-order queues
        for (auto iter = event_list.rbegin(); iter != event_list.rend(); ++iter) {
            iter->ev->Wait();
        }
    }

    IterationInfo AddIteration(
        EventList& events, FixtureFamilyResult& ff_result, FixtureResult& fixture_result) {
        WaitForEventList(events);
        IterationInfo iter_info;
        if (ff_result.steps.size() > std::numeric_limits<int>::max()) {
            throw std::invalid_argument("Fixture family has too many steps");
        }

        for (auto& ev_info : events) {
            // If step is new, insert a new step info
            ff_result.steps.emplace(
                ev_info.step_name, StepInfo{static_cast<int>(ff_result.steps.size())});

            // Insert iteration duration
            iter_info.durations.emplace(ev_info.step_name, ev_info.ev->GetDuration());
        }
        fixture_result.iterations.push_back(iter_info);
        return iter_info;
    }
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_FIXTURE_RUNNER_H_
