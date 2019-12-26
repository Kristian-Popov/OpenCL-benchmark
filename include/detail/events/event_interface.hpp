#ifndef KPV_EVENTS_EVENT_INTERFACE_H_
#define KPV_EVENTS_EVENT_INTERFACE_H_

#include "duration.hpp"

namespace kpv {
namespace cl_benchmark {
class EventInterface {
public:
    virtual Duration GetDuration() = 0;
    virtual void Wait() = 0;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_EVENTS_EVENT_INTERFACE_H_
