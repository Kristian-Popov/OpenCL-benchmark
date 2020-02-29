#ifndef KPV_FIXTURES_FIXTURE_FAMILY_H_
#define KPV_FIXTURES_FIXTURE_FAMILY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "boost/optional.hpp"
#include "detail/fixtures/fixture.hpp"
#include "detail/fixtures/fixture_id.hpp"

namespace kpv {
namespace cl_benchmark {
struct FixtureFamily {
    std::string name;
    std::unordered_map<FixtureId, std::shared_ptr<Fixture>> fixtures;
    boost::optional<int32_t> element_count;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_FIXTURES_FIXTURE_FAMILY_H_
