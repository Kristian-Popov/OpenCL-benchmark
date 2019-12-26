#ifndef KPV_DEVICES_DEVICE_INTERFACE_H_
#define KPV_DEVICES_DEVICE_INTERFACE_H_

#include <memory>
#include <string>
#include <vector>

namespace kpv {
namespace cl_benchmark {
class PlatformInterface;

class DeviceInterface {
public:
    virtual std::string Name() = 0;
    virtual std::vector<std::string> Extensions() = 0;
    virtual std::string UniqueName() = 0;
    virtual std::weak_ptr<PlatformInterface> platform() = 0;
    virtual ~DeviceInterface() noexcept {}
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_DEVICES_DEVICE_INTERFACE_H_
