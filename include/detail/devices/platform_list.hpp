#ifndef KPV_DEVICES_PLATFORM_LIST_H_
#define KPV_DEVICES_PLATFORM_LIST_H_

#include <boost/compute.hpp>
#include <memory>
#include <vector>

#include "devices/opencl_platform.hpp"
#include "devices/platform_interface.hpp"
#include "run_settings.hpp"

namespace kpv {
namespace cl_benchmark {
class PlatformList {
public:
    PlatformList(const DeviceConfiguration& device_config) {
        std::vector<boost::compute::platform> opencl_platforms =
            boost::compute::system::platforms();
        opencl_platforms_.reserve(opencl_platforms.size());
        for (boost::compute::platform& platform : opencl_platforms) {
            auto& ptr = std::make_shared<OpenClPlatform>(platform);
            ptr->PopulateDeviceList(device_config);
            all_platforms_.push_back(ptr);
            opencl_platforms_.push_back(ptr);
        }
        // TODO add host platform
    }

    std::vector<std::shared_ptr<PlatformInterface>> OpenClPlatforms() const {
        return opencl_platforms_;
    }

    std::vector<std::shared_ptr<PlatformInterface>> AllPlatforms() const { return all_platforms_; }

private:
    std::vector<std::shared_ptr<PlatformInterface>> all_platforms_;
    std::vector<std::shared_ptr<PlatformInterface>> opencl_platforms_;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_DEVICES_PLATFORM_LIST_H_
