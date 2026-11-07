#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer
{
    using timep_t = typename std::chrono::steady_clock::time_point;
    timep_t _start = std::chrono::steady_clock::now(), _end = {};

public:
    void tick() {
        _end = timep_t{};
        _start = std::chrono::steady_clock::now();
    }

    void tock() { _end = std::chrono::steady_clock::now(); }

    double duration() const {
        //gsl_Expects(_end != timep_t{} && "toc before reporting");
        return std::chrono::duration<double>(_end - _start).count();
    }
};

#endif