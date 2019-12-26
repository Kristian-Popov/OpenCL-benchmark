#ifndef KPV_DURATION_H_
#define KPV_DURATION_H_

#include <chrono>
#include <string>

#include "nlohmann/json.hpp"

namespace kpv {
namespace cl_benchmark {
class Duration {
public:
    /*
    Internally duration is stored as double-precision floating point count of nanoseconds.
    OpenCL uses cl_ulong (same as uint64_t).
    It is perfect to specify duration of the whole operation, but it is not
    good when divided by very large number (e.g. 9mcs=900 000 ns divided by 1M elements
    is just 0 ns per element, when using floating point we get approx. 0.9 ns per element,
    which may bring more useful information).
    Storing it in nanoseconds improves accurancy greatly, max duration that fits
    without losing accurancy is 52 days, so more than enough.
    */
    typedef std::chrono::duration<double, std::nano> InternalType;

    constexpr Duration() noexcept : duration_(InternalType::zero()) {}

    template <typename D>
    explicit Duration(const D& duration)
        : duration_(std::chrono::duration_cast<InternalType>(duration)) {
        if (duration < D::zero()) {
            throw std::invalid_argument("Attempt to construct a negative duration.");
        }
    }

    explicit Duration(const InternalType& duration) : duration_(duration) {
        if (duration < InternalType::zero()) {
            throw std::invalid_argument("Attempt to construct a negative duration.");
        }
    }

    Duration& operator=(const Duration& d) {
        this->duration_ = d.duration_;
        return *this;
    }

    template <typename D>
    Duration& operator=(D duration) {
        if (duration < D::zero()) {
            throw std::invalid_argument("Attempt to construct a negative duration.");
        }
        this->duration_ = std::chrono::duration_cast<InternalType>(duration);
        return *this;
    }

    Duration& operator+=(const Duration& d) {
        duration_ += d.duration_;
        return *this;
    }

    template <typename T>
    Duration& operator*=(T m) {
        static_assert(
            std::is_arithmetic<T>::value,
            "Duration operator*= second argument is not an arithmetic value.");
        duration_ *= m;
        return *this;
    }

    template <typename T>
    Duration& operator/=(T m) {
        static_assert(
            std::is_arithmetic<T>::value,
            "Duration operator/= second argument is not an arithmetic value.");
        duration_ /= m;
        return *this;
    }

    double AsSeconds() const {
        return std::chrono::duration_cast<std::chrono::duration<double>>(duration_).count();
    }

    bool operator==(const Duration& rhs) const noexcept { return duration_ == rhs.duration_; }

    bool operator!=(const Duration& rhs) const noexcept { return !(*this == rhs); }

    std::chrono::duration<double, std::nano> duration() const noexcept { return duration_; }

    friend Duration operator+(Duration lhs, Duration rhs) {
        lhs += rhs;
        return lhs;
    }

    bool operator<(const Duration& rhs) const noexcept { return duration_ < rhs.duration_; }
    bool operator<=(const Duration& rhs) const noexcept { return duration_ <= rhs.duration_; }
    bool operator>(const Duration& rhs) const noexcept { return duration_ > rhs.duration_; }
    bool operator>=(const Duration& rhs) const noexcept { return duration_ >= rhs.duration_; }

    static const Duration Min() noexcept {
        // double::min() duration is negative but duration below zero makes no sense
        // Using zero instead
        return Duration(InternalType::zero());
    }

    static const Duration Max() noexcept { return Duration(InternalType::max()); }

    template <typename T>
    friend Duration operator*(Duration lhs, T rhs) {
        lhs *= rhs;
        return lhs;
    }

    template <typename T>
    friend Duration operator/(Duration lhs, T rhs) {
        lhs /= rhs;
        return lhs;
    }

    friend double operator/(Duration lhs, Duration rhs) { return lhs.duration_ / rhs.duration_; }

private:
    InternalType duration_;
};

inline void to_json(nlohmann::json& j, const kpv::cl_benchmark::Duration& p) {
    j = nlohmann::json::object({{"durationDoubleNs", p.duration().count()}});
}

inline void from_json(const nlohmann::json& j, kpv::cl_benchmark::Duration& p) {
    double val{0.0};
    j.at("durationDoubleNs").get_to(val);  // TODO get rid of durationDoubleNs???
    p = kpv::cl_benchmark::Duration{std::chrono::duration<double, std::nano>(val)};
}
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_DURATION_H_
