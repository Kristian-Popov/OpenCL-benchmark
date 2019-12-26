#include <functional>
#include <memory>

#include "cl_benchmark_main.hpp"
#include "fixtures/factorial_opencl_fixture.h"

using namespace kpv::cl_benchmark;

FixtureFamily CreateFactorialFixture(const PlatformList& platform_list, int32_t data_size) {
    FixtureFamily fixture_family;
    fixture_family.name = (boost::format("Factorial, %1% elements") % data_size).str();
    fixture_family.element_count = data_size;
    for (auto& platform : platform_list.OpenClPlatforms()) {
        for (auto& device : platform->GetDevices()) {
            fixture_family.fixtures.insert(
                std::make_pair<const FixtureId, std::shared_ptr<Fixture>>(
                    FixtureId(fixture_family.name, device, ""),
                    std::make_shared<kpv::FactorialOpenClFixture>(
                        std::dynamic_pointer_cast<OpenClDevice>(device), data_size)));
        }
    }
    return fixture_family;
}

using namespace ::std::placeholders;

REGISTER_FIXTURE("trivial-factorial", std::bind(&CreateFactorialFixture, _1, 100));
REGISTER_FIXTURE("trivial-factorial", std::bind(&CreateFactorialFixture, _1, 1000));
REGISTER_FIXTURE("trivial-factorial", std::bind(&CreateFactorialFixture, _1, 100000));
REGISTER_FIXTURE("trivial-factorial", std::bind(&CreateFactorialFixture, _1, 1000000));
