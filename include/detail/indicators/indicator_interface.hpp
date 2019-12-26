#ifndef KPV_INDICATORS_INDICATOR_INTERFACE_H_
#define KPV_INDICATORS_INDICATOR_INTERFACE_H_

#include <map>
#include <memory>

namespace kpv {
namespace cl_benchmark {
class IndicatorInterface {
public:
    // TODO can we replace that with to_json?
    virtual void SerializeValue(nlohmann::json& tree) = 0;

    virtual ~IndicatorInterface() noexcept {}
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_INDICATORS_INDICATOR_INTERFACE_H_
