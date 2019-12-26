#ifndef KPV_EVENTS_EVENT_LIST_H_
#define KPV_EVENTS_EVENT_LIST_H_

#include <memory>
#include <vector>

#include "boost/compute.hpp"
#include "opencl_event.hpp"

namespace kpv {
namespace cl_benchmark {
class EventList {
public:
    struct EventInfo {
        std::string step_name;
        std::unique_ptr<EventInterface> ev;
    };

    typedef EventInfo value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::vector<EventInfo>::const_iterator const_iterator;
    typedef std::vector<EventInfo>::iterator iterator;
    typedef std::vector<EventInfo>::reverse_iterator reverse_iterator;

    void AddOpenClEvent(const std::string& step_name, boost::compute::event& e) {
        events_.push_back({step_name, std::make_unique<OpenClEvent>(e)});
    }

    template <typename T>
    void AddOpenClEvent(const std::string& step_name, boost::compute::future<T>& e) {
        AddOpenClEvent(step_name, e.get_event());
    }

    const_iterator cbegin() const { return events_.cbegin(); }

    const_iterator cend() const { return events_.cend(); }

    iterator begin() { return events_.begin(); }

    iterator end() { return events_.end(); }

    reverse_iterator rbegin() { return events_.rbegin(); }

    reverse_iterator rend() { return events_.rend(); }

    std::size_t size() const { return events_.size(); }

private:
    std::vector<EventInfo> events_;
};
}  // namespace cl_benchmark
}  // namespace kpv

#endif  // KPV_EVENTS_EVENT_LIST_H_
