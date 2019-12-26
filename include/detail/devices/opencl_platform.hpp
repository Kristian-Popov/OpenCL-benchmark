#ifndef KPV_OPENCL_PLATFORM_H_
#define KPV_OPENCL_PLATFORM_H_

#include "boost/compute.hpp"
#include "opencl_device.hpp"
#include "platform_interface.hpp"
#include "run_settings.hpp"

namespace kpv {
namespace cl_benchmark {
class OpenClPlatform : public PlatformInterface,
                       public std::enable_shared_from_this<OpenClPlatform> {
public:
    OpenClPlatform(const boost::compute::platform& platform) : opencl_platform_(platform) {}

    void PopulateDeviceList(const DeviceConfiguration& device_config) {
        // Devices need a week pointer to platform, so this cannot be done in a constructor
        std::vector<boost::compute::device> devices;
        if (device_config.cpu_opencl_devices) {
            ConcatVectors(devices, opencl_platform_.devices(CL_DEVICE_TYPE_CPU));
        }
        if (device_config.gpu_opencl_devices) {
            ConcatVectors(devices, opencl_platform_.devices(CL_DEVICE_TYPE_GPU));
        }
        if (device_config.other_opencl_devices) {
            ConcatVectors(
                devices,
                opencl_platform_.devices(CL_DEVICE_TYPE_ACCELERATOR || CL_DEVICE_TYPE_CUSTOM));
        }

        // Device may belong to more than one class, so remove duplicates
        // TODO add a unit test for this
        std::sort(
            devices.begin(), devices.end(),
            [](const boost::compute::device& lhs, const boost::compute::device& rhs) {
                return lhs.id() < rhs.id();
            });
        devices.erase(std::unique(devices.begin(), devices.end()), devices.end());

        devices_.reserve(devices.size());
        for (boost::compute::device& device : devices) {
            devices_.push_back(std::make_shared<OpenClDevice>(device, shared_from_this()));
        }
    }

    std::string Name() override { return opencl_platform_.name(); }

    std::vector<std::shared_ptr<DeviceInterface>> GetDevices() override {
        return std::vector<std::shared_ptr<DeviceInterface>>(devices_.cbegin(), devices_.cend());
    }

private:
    boost::compute::platform opencl_platform_;
    std::vector<std::shared_ptr<OpenClDevice>> devices_;

    template <typename T>
    void ConcatVectors(T& v1, const T& v2) {
        v1.insert(v1.end(), v2.begin(), v2.end());
    }
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_OPENCL_PLATFORM_H_
