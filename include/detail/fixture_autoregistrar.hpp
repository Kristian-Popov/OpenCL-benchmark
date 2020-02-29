#ifndef KPV_FIXTURE_AUTOREGISTRAR_H_
#define KPV_FIXTURE_AUTOREGISTRAR_H_

#include "detail/fixture_registry.hpp"

namespace kpv {
namespace cl_benchmark {
class FixtureAutoregistrar {
public:
    typedef std::function<FixtureFamily(const PlatformList&)> FixtureFactory;
    FixtureAutoregistrar(const std::string& category_id, FixtureFactory fixture_factory) {
        std::shared_ptr<FixtureRegistry> fixture_registry = FixtureRegistry::instance().lock();
        if (!fixture_registry) {
            throw std::runtime_error("Fixture registry was not constructed.");
        }
        fixture_registry->Register(category_id, fixture_factory);
    }
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_FIXTURE_AUTOREGISTRAR_H_
