#pragma once

#include <memory>

#include "cl_benchmark.hpp"

namespace kpv {
class FactorialOpenClFixture final : public cl_benchmark::Fixture {
public:
    FactorialOpenClFixture(
        const std::shared_ptr<cl_benchmark::OpenClDevice>& device, int data_size);

    virtual void Initialize() override;

    kpv::cl_benchmark::EventList Execute(const cl_benchmark::RuntimeParams& params) override;

    virtual void VerifyResults() override;

    virtual ~FactorialOpenClFixture() noexcept {}

private:
    const int data_size_;
    std::vector<cl_int> input_data_;
    std::vector<cl_ulong> expected_output_data_;
    std::vector<cl_ulong> output_data_;
    boost::compute::kernel kernel_;
    const std::shared_ptr<cl_benchmark::OpenClDevice> device_;
    static const std::unordered_map<cl_int, cl_ulong> correct_factorial_values_;

    void GenerateData();
};

}  // namespace kpv
