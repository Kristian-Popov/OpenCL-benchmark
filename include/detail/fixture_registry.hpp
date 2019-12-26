#ifndef KPV_FIXTURE_REGISTRY_H_
#define KPV_FIXTURE_REGISTRY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "devices/platform_list.hpp"
#include "fixtures/fixture_family.hpp"

namespace kpv {
namespace cl_benchmark {
// TODO forbid construction outside allowed context if possible
class FixtureRegistry {
public:
    FixtureRegistry(const FixtureRegistry&) = delete;
    FixtureRegistry(FixtureRegistry&&) = delete;

    FixtureRegistry& operator=(const FixtureRegistry&) = delete;
    FixtureRegistry& operator=(FixtureRegistry&) = delete;

    typedef std::function<FixtureFamily(const PlatformList&)> FixtureFactory;
    typedef std::vector<std::pair<std::string, FixtureFactory>> FactoryList;
    void Register(const std::string& category_id, FixtureFactory fixture_factory) {
        factories_.emplace_back(category_id, fixture_factory);
    }

    // We need a singleton so macros can register factories using this global instance
    static std::weak_ptr<FixtureRegistry> instance() {
        static auto instance = std::shared_ptr<FixtureRegistry>(new FixtureRegistry);
        return instance;
    }

    std::vector<std::string> GetAllCategories() {
        std::vector<std::string> result;
        // Copy all category names
        std::transform(
            factories_.cbegin(), factories_.cend(), std::back_inserter(result),
            [](const value_type& v) { return v.first; });
        // Sort category name list
        std::sort(result.begin(), result.end());
        // Remove duplicate items
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    FactoryList::iterator begin() { return factories_.begin(); }

    FactoryList::iterator end() { return factories_.end(); }

    FactoryList::const_iterator cbegin() const { return factories_.cbegin(); }

    FactoryList::const_iterator cend() const { return factories_.cend(); }

    typedef std::string key_type;
    typedef FixtureFactory mapped_type;
    typedef std::pair<const std::string, FixtureFactory> value_type;

private:
    FixtureRegistry() {}

    FactoryList factories_;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_FIXTURE_REGISTRY_H_
