#ifndef KPV_DEVICES_PLATFORM_INTERFACE_H_
#define KPV_DEVICES_PLATFORM_INTERFACE_H_

#include <memory>
#include <string>
#include <vector>

#include "device_interface.hpp"

namespace kpv {
namespace cl_benchmark {
class PlatformInterface {
public:
    virtual std::string Name() = 0;
    virtual std::vector<std::shared_ptr<DeviceInterface>> GetDevices() = 0;
    virtual ~PlatformInterface() noexcept {}
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_DEVICES_PLATFORM_INTERFACE_H_
