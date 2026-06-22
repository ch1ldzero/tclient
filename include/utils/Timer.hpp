#pragma once

#include <chrono>
#include <mutex>

namespace tclient {

class Timer {
public:
    using clock = std::chrono::steady_clock;

    Timer();

    void Start();
    void Pause();
    void Resume();
    void Stop();
    void Reset();

    std::chrono::nanoseconds Elapsed() const;

private:
    mutable std::mutex mutex;

    bool is_running = false;
    bool is_paused = false;

    clock::time_point start_point;
    std::chrono::nanoseconds elapsed_time = std::chrono::nanoseconds(0);
};

} // namespace tclient

