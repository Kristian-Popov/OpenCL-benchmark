#include "factorial_opencl_fixture.h"

#include <boost/format.hpp>
#include <random>

namespace {
const char* kProgramCode = R"(
ulong FactorialImplementation(int val)
{
    ulong result = 1;
    for(int i = 1; i <= val; i++)
    {
        result *= i;
    }
    return result;
}

__kernel void TrivialFactorial(__global int* input, __global ulong* output)
{
    size_t id = get_global_id(0);
    output[id] = FactorialImplementation(input[id]);
}
)";

constexpr const char* const kCompilerOptions = "-Werror";
}  // namespace

namespace kpv {
FactorialOpenClFixture::FactorialOpenClFixture(
    const std::shared_ptr<cl_benchmark::OpenClDevice>& device, int data_size)
    : device_(device), data_size_(data_size) {}

void FactorialOpenClFixture::Initialize() {
    GenerateData();
    auto program = boost::compute::program::build_with_source(
        kProgramCode, device_->GetContext(), kCompilerOptions);
    kernel_ = program.create_kernel("TrivialFactorial");
}

kpv::cl_benchmark::EventList FactorialOpenClFixture::Execute(
    const cl_benchmark::RuntimeParams& params) {
    boost::compute::context& context = device_->GetContext();
    boost::compute::command_queue& queue = device_->GetQueue();

    kpv::cl_benchmark::EventList event_list;

    // create a vector on the device
    boost::compute::vector<cl_int> input_device_vector(data_size_, context);

    // copy data from the host to the device
    event_list.AddOpenClEvent(
        "Copying input data",
        boost::compute::copy_async(
            input_data_.begin(), input_data_.end(), input_device_vector.begin(), queue));

    boost::compute::vector<unsigned long long> output_device_vector(data_size_, context);
    kernel_.set_arg(0, input_device_vector);
    kernel_.set_arg(1, output_device_vector);

    event_list.AddOpenClEvent(
        "Calculating", queue.enqueue_1d_range_kernel(kernel_, 0, data_size_, 0));

    output_data_.resize(data_size_);
    event_list.AddOpenClEvent(
        "Copying output data",
        boost::compute::copy_async(
            output_device_vector.begin(), output_device_vector.end(), output_data_.begin(), queue));

    return event_list;
}

void FactorialOpenClFixture::VerifyResults() {
    if (output_data_.size() != expected_output_data_.size()) {
        throw std::runtime_error(
            "Result verification has failed for factorial fixture . "
            "Output data count is another from expected one.");
    }
    auto mismatched_values = std::mismatch(
        output_data_.cbegin(), output_data_.cend(), expected_output_data_.cbegin(),
        expected_output_data_.cend());
    if (mismatched_values.first != output_data_.cend()) {
        cl_ulong max_abs_error = *mismatched_values.first - *mismatched_values.second;
        throw std::runtime_error(
            (boost::format("Result verification has failed for trivial factorial fixture. "
                           "Maximum absolute error is %1% for input value %2% "
                           "(exact equality is expected).") %
             max_abs_error % *mismatched_values.first)
                .str());
    }
}

void FactorialOpenClFixture::GenerateData() {
    const cl_int min_input_val = 0;
    const cl_int max_input_val =
        20;  // Max value whose factorial fits into 64-bit unsigned integer number.

    std::random_device random_dev;
    std::mt19937 gen(random_dev());
    std::uniform_int_distribution<cl_int> uniform_dist(min_input_val, max_input_val);

    input_data_.resize(data_size_);
    std::generate_n(
        input_data_.begin(), data_size_, [&gen, &uniform_dist]() { return uniform_dist(gen); });

    expected_output_data_.reserve(data_size_);
    std::transform(
        input_data_.begin(), input_data_.end(), std::back_inserter(expected_output_data_),
        [this](cl_int i) { return correct_factorial_values_.at(i); });
}

const std::unordered_map<cl_int, cl_ulong> FactorialOpenClFixture::correct_factorial_values_ = {
    {0, 1ull},
    {1, 1ull},
    {2, 2ull},
    {3, 6ull},
    {4, 24ull},
    {5, 120ull},
    {6, 720ull},
    {7, 5040ull},
    {8, 40320ull},
    {9, 362880ull},
    {10, 3628800ull},
    {11, 39916800ull},
    {12, 479001600ull},
    {13, 6227020800ull},
    {14, 87178291200ull},
    {15, 1307674368000ull},
    {16, 20922789888000ull},
    {17, 355687428096000ull},
    {18, 6402373705728000ull},
    {19, 121645100408832000ull},
    {20, 2432902008176640000ull},
};
}  // namespace kpv
