#include "trail.hpp"

Trail::Trail(Clock::time_point departed_at): departed_at_{departed_at} {}

void Trail::update(Clock::time_point departed_at) {
    departed_at_ = departed_at;
}

bool Trail::expired(Clock::time_point now, std::chrono::seconds lifetime) const {
    return now - departed_at_ >= lifetime;
}

int Trail::age_seconds(Clock::time_point now, std::chrono::seconds lifetime) const {
    if (expired(now, lifetime)) {
        return -1;
    }
    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(now - departed_at_).count());
}
