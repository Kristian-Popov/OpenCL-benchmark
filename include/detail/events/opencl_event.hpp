#ifndef KPV_EVENTS_OPENCL_EVENT_H_
#define KPV_EVENTS_OPENCL_EVENT_H_

#include "boost/compute.hpp"
#include "detail/events/event_interface.hpp"

namespace kpv {
namespace cl_benchmark {
class OpenClEvent : public EventInterface {
public:
    OpenClEvent(boost::compute::event& e) : event_(e) {}

    virtual Duration GetDuration() override {
        return Duration{event_.duration<Duration::InternalType>()};
    }

    virtual void Wait() override { event_.wait(); }

private:
    boost::compute::event event_;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_EVENTS_OPENCL_EVENT_H_
