#include <chrono>

#include "catch.hpp"
#include "detail/duration.hpp"

TEST_CASE("Default-constructed duration is same as zero-initialized duration", "[duration]") {
    using namespace kpv::cl_benchmark;
    using namespace std::literals::chrono_literals;
    REQUIRE(Duration() == Duration(0ns));
    REQUIRE(Duration().duration() == Duration(0ns).duration());
    REQUIRE(Duration().AsSeconds() == Duration(0ns).AsSeconds());
}

TEST_CASE("Duration's constructor from internal type works well", "[duration]") {
    using namespace kpv::cl_benchmark;
    double ns = 3.5;
    auto internal_duration = std::chrono::duration<double, std::nano>{ns};
    auto duration = Duration{internal_duration};
    REQUIRE(duration.duration() == internal_duration);
    REQUIRE(duration.AsSeconds() == Approx(ns * 1e-9));
}

TEST_CASE("Duration's another duration constructor works well", "[duration]") {
    using namespace kpv::cl_benchmark;
    double ns = 3.5;
    auto val = std::chrono::duration<double>{ns * 1e-9};
    auto duration = Duration{val};
    REQUIRE(duration.AsSeconds() == Approx(ns * 1e-9));
}

TEST_CASE("Duration assignment operator works well", "[duration]") {
    using namespace kpv::cl_benchmark;
    double ns = 3.5;
    auto internal_duration = std::chrono::duration<double, std::nano>{ns};
    auto duration = Duration{internal_duration};
    REQUIRE(duration.AsSeconds() == Approx(ns * 1e-9));
    duration = duration;  // Self-assignment
    REQUIRE(duration.AsSeconds() == Approx(ns * 1e-9));
    duration = Duration();
    REQUIRE(duration.AsSeconds() == 0);
}
