#ifndef KPV_FIXTURES_FIXTURE_ID_H_
#define KPV_FIXTURES_FIXTURE_ID_H_

#include <memory>
#include <string>

#include "detail/devices/device_interface.hpp"

namespace kpv {
namespace cl_benchmark {
class FixtureId {
public:
    // TODO process family name at all?
    FixtureId(
        const std::string& family_name, const std::shared_ptr<DeviceInterface>& device,
        const std::string& algorithm = "")
        : family_name_(family_name), device_(device), algorithm_(algorithm) {}

    bool operator<(const FixtureId& rhs) const {
        // First sort by family name
        // TODO move this generic comparison logic to a separate function
        if (family_name_ < rhs.family_name_) {
            return true;
        } else if (rhs.family_name_ < family_name_) {
            return false;
        }

        if (device_ < rhs.device_) {
            return true;
        } else if (rhs.device_ < device_) {
            return false;
        }

        if (algorithm_ < rhs.algorithm_) {
            return true;
        }
        return false;
    }

    bool operator==(const FixtureId& rhs) const {
        return family_name_ == rhs.family_name_ && device_ == rhs.device_ &&
               algorithm_ == rhs.algorithm_;
    }

    bool operator!=(const FixtureId& rhs) const { return !(*this == rhs); }

    std::string family_name() const { return family_name_; }

    // TODO replace with weak_ptr
    std::shared_ptr<DeviceInterface> device() const { return device_; }

    std::string algorithm() const { return algorithm_; }

    std::string Serialize() const {
        std::string result = device_->UniqueName();
        if (!algorithm_.empty()) {
            result += ", " + algorithm_;
        }
        return result;
    }

private:
    std::string family_name_;
    std::shared_ptr<DeviceInterface> device_;
    std::string algorithm_;
};
}  // namespace cl_benchmark
}  // namespace kpv

namespace std {
template <>
struct hash<kpv::cl_benchmark::FixtureId> {
    size_t operator()(const kpv::cl_benchmark::FixtureId& f) const {
        // Based on https://stackoverflow.com/a/27952689
        // TODO is this function any good?
        size_t result = 0;
        size_t family_name_hash = hash<std::string>()(f.family_name());
        size_t device_hash =
            hash<std::shared_ptr<kpv::cl_benchmark::DeviceInterface>>()(f.device());
        size_t algorithm_hash = hash<std::string>()(f.algorithm());
        return (family_name_hash << 2) + (family_name_hash << 1) + (device_hash << 1) +
               device_hash + algorithm_hash;
    }
};
}  // namespace std

#endif  // KPV_FIXTURES_FIXTURE_ID_H_
