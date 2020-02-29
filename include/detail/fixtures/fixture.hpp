#ifndef KPV_FIXTURES_FIXTURE_H_
#define KPV_FIXTURES_FIXTURE_H_

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "detail/devices/device_interface.hpp"
#include "detail/duration.hpp"
#include "detail/events/event_list.hpp"

namespace kpv {
namespace cl_benchmark {

struct RuntimeParams {
    std::string additional_params;
};

class Fixture {
public:
    /*
    Optional method to initialize a fixture.
    Called exactly once before running a fixture.
    Memory allocations should be done here to avoid excess memory consumption since many
    fixtures may be created at once, but only one of them will be executed at once.
    */
    virtual void Initialize() {}

    /*
    Get a list of required extensions required by this fixture. Override this if fixture requires
    any extensions.
    */
    virtual std::vector<std::string> GetRequiredExtensions() { return std::vector<std::string>(); }

    virtual EventList Execute(const RuntimeParams& params) = 0;

    /*
    Optional method to finalize a fixture.
    Called exactly once after running all iterations.
    */
    virtual void Finalize() {}

    /*
    Verify execution results
    Called exactly once but for every platform/device.
    */
    virtual void VerifyResults() {}

    virtual std::string Algorithm() { return std::string(); }

    /*
    Store results of fixture to a persistent storage (e.g. graphic file).
    Every fixture may provide its own method, but it is optional.
    */
    virtual void StoreResults() {}

    virtual ~Fixture() noexcept {}
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_FIXTURES_FIXTURE_H_
