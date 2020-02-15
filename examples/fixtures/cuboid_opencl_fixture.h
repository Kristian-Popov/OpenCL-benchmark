#ifndef EXAMPLES_FIXTURES_CUBOID_OPENCL_FIXTURE_H_
#define EXAMPLES_FIXTURES_CUBOID_OPENCL_FIXTURE_H_

#include <memory>

#include "cl_benchmark.hpp"

namespace kpv {
template <typename T>
class CuboidOpenClFixture final : public cl_benchmark::Fixture {
public:
    // data_size is amount of cuboids that are processed
    CuboidOpenClFixture(const std::shared_ptr<cl_benchmark::OpenClDevice>& device, int data_size);

    std::vector<std::string> GetRequiredExtensions() override;

    virtual void Initialize() override;

    kpv::cl_benchmark::EventList Execute(const cl_benchmark::RuntimeParams& params) override;

    virtual ~CuboidOpenClFixture() noexcept {}

private:
    const int data_size_;
    std::vector<T> dimensions_;
    std::vector<T> volumes_;
    std::vector<T> surfaces_;
    boost::compute::kernel kernel_;
    const std::shared_ptr<cl_benchmark::OpenClDevice> device_;
    static constexpr T min_len = static_cast<T>(1e-6);  // Minimum value used for all dimensions
    static constexpr T max_len = static_cast<T>(1e6);   // Maximum value used for all dimensions

    void GenerateData();
};

template class CuboidOpenClFixture<float>;
template class CuboidOpenClFixture<double>;

}  // namespace kpv

#endif  // EXAMPLES_FIXTURES_CUBOID_OPENCL_FIXTURE_H_
