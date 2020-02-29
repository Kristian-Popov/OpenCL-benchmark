#include "cuboid_opencl_fixture.h"

#include <boost/format.hpp>
#include <random>

namespace {
const char* kProgramCode = R"(
T Cuboid_Volume(__global T* dimensions)
{
    return dimensions[0] * dimensions[1] * dimensions[2];
}

T Cuboid_Surface(__global T* dimensions)
{
    return 2 * (dimensions[0] * dimensions[1] + dimensions[1] * dimensions[2] + dimensions[0] * dimensions[2]);
}

__kernel void CuboidVolumesAndSurfaces(__global T* dimensions, __global T* volumes, __global T* surfaces)
{
    size_t id = get_global_id(0);
    volumes[id] = Cuboid_Volume(dimensions + 3*id);
    surfaces[id] = Cuboid_Surface(dimensions + 3*id);
}
)";

template <typename T>
struct OpenClTypeTraits {
    static const char* const required_extension;
};

// No extensions needed for single precision arithmetic
template <>
const char* const OpenClTypeTraits<float>::required_extension = "";
template <>
const char* const OpenClTypeTraits<double>::required_extension = "cl_khr_fp64";

constexpr const char* const kCompilerOptions = "-Werror";

boost::compute::program BuildProgram(
    const std::string& source, boost::compute::context& context,
    const std::string& buildOptions = std::string()) {
    // Taken from boost::compute::program::create_with_source() so we have build log
    // left in case of errors
    const char* source_string = source.c_str();

    cl_int error = 0;
    cl_program program_ = clCreateProgramWithSource(context, 1, &source_string, 0, &error);
    boost::compute::program program;
    try {
        if (!program_) {
            throw boost::compute::opencl_error(error);
        }
        program = boost::compute::program(program_);
        program.build(buildOptions);
        return program;
    } catch (boost::compute::opencl_error& error) {
        if (error.error_code() == CL_BUILD_PROGRAM_FAILURE) {
            BOOST_LOG_TRIVIAL(error) << "OpenCL program build failure: " << program.build_log();
        }
        throw;
    }
}
}  // namespace

namespace kpv {
template <typename T>
CuboidOpenClFixture<T>::CuboidOpenClFixture(
    const std::shared_ptr<cl_benchmark::OpenClDevice>& device, int data_size)
    : device_(device), data_size_(data_size) {}

template <>
void CuboidOpenClFixture<float>::Initialize() {
    GenerateData();
    std::string compiler_options = kCompilerOptions;
    compiler_options += " -DT=float";

    auto program = BuildProgram(kProgramCode, device_->GetContext(), compiler_options);
    kernel_ = program.create_kernel("CuboidVolumesAndSurfaces");
}

template <>
void CuboidOpenClFixture<double>::Initialize() {
    GenerateData();
    std::string compiler_options = kCompilerOptions;
    compiler_options += " -DT=double";

    std::string source = R"(
#if __OPENCL_VERSION__ <= CL_VERSION_1_1
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif)";
    source += kProgramCode;
    auto program = BuildProgram(source, device_->GetContext(), compiler_options);
    kernel_ = program.create_kernel("CuboidVolumesAndSurfaces");
}

template <typename T>
std::vector<std::string> CuboidOpenClFixture<T>::GetRequiredExtensions() {
    std::string required_extension = OpenClTypeTraits<T>::required_extension;
    std::vector<std::string> result;
    if (!required_extension.empty()) {
        result.push_back(required_extension);
    }
    return result;
}

template <typename T>
kpv::cl_benchmark::EventList CuboidOpenClFixture<T>::Execute(
    const cl_benchmark::RuntimeParams& params) {
    boost::compute::context& context = device_->GetContext();
    boost::compute::command_queue& queue = device_->GetQueue();

    kpv::cl_benchmark::EventList event_list;

    // create a vector on the device
    boost::compute::vector<T> input_device_vector(3 * data_size_, context);
    boost::compute::vector<T> output_volumes_vector(data_size_, context);
    boost::compute::vector<T> output_surfaces_vector(data_size_, context);

    // Map input data, copy them and unmap
    {
        boost::compute::event event;  // Mapping is blocking
        void* input_ptr = queue.enqueue_map_buffer(
            input_device_vector.get_buffer(), CL_MAP_READ, 0,
            input_device_vector.size() * sizeof(T), event);
        event_list.AddOpenClEvent("Map input data", event);

        T* input_ptr_casted = reinterpret_cast<T*>(input_ptr);
        // TODO include time spent on this, needs host timer
        std::copy(std::cbegin(dimensions_), std::cend(dimensions_), input_ptr_casted);

        event_list.AddOpenClEvent(
            "Unmap input data",
            queue.enqueue_unmap_buffer(input_device_vector.get_buffer(), input_ptr));
    }

    kernel_.set_arg(0, input_device_vector);
    kernel_.set_arg(1, output_volumes_vector);
    kernel_.set_arg(2, output_surfaces_vector);

    event_list.AddOpenClEvent(
        "Calculating", queue.enqueue_1d_range_kernel(kernel_, 0, data_size_, 0));

    // Map volumes data, copy them and unmap
    {
        boost::compute::event event;  // Mapping is blocking
        void* ptr = queue.enqueue_map_buffer(
            output_volumes_vector.get_buffer(), CL_MAP_WRITE, 0, data_size_ * sizeof(T), event);
        event_list.AddOpenClEvent("Map output volume data", event);

        const T* ptr_casted = reinterpret_cast<const T*>(ptr);
        // TODO include time spent on this, needs host timer
        volumes_.reserve(data_size_);
        std::copy_n(ptr_casted, data_size_, std::back_inserter(volumes_));

        event_list.AddOpenClEvent(
            "Unmap output volume data",
            queue.enqueue_unmap_buffer(output_volumes_vector.get_buffer(), ptr));
    }
    // Map surface buffer, copy them and unmap
    {
        boost::compute::event event;  // Mapping is blocking
        void* ptr = queue.enqueue_map_buffer(
            output_surfaces_vector.get_buffer(), CL_MAP_WRITE, 0, data_size_ * sizeof(T), event);
        event_list.AddOpenClEvent("Map output surface data", event);

        const T* ptr_casted = reinterpret_cast<const T*>(ptr);
        // TODO include time spent on this, needs host timer
        surfaces_.reserve(data_size_);
        std::copy_n(ptr_casted, data_size_, std::back_inserter(surfaces_));

        event_list.AddOpenClEvent(
            "Unmap output surface data",
            queue.enqueue_unmap_buffer(output_surfaces_vector.get_buffer(), ptr));
    }

    return event_list;
}

template <typename T>
void CuboidOpenClFixture<T>::GenerateData() {
    std::random_device random_dev;
    std::mt19937 gen(random_dev());
    std::uniform_real_distribution<T> uniform_dist(min_len, max_len);

    // data_size_ is amount of cuboids (i.e. triples of dimensions), so amount of values is 3 times
    // bigger
    int val_count = data_size_ * 3;
    dimensions_.resize(val_count);
    std::generate_n(
        dimensions_.begin(), val_count, [&gen, &uniform_dist]() { return uniform_dist(gen); });
}
}  // namespace kpv
