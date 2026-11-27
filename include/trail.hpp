#pragma once

#include <chrono>

class Trail {
    public:
        using Clock = std::chrono::steady_clock;

        explicit Trail(Clock::time_point departed_at);
        void update(Clock::time_point departed_at);
        bool expired(Clock::time_point now, std::chrono::seconds lifetime) const;
        int age_seconds(Clock::time_point now, std::chrono::seconds lifetime) const;

    private:
        Clock::time_point departed_at_;
};
