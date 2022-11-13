#pragma once
#include "timer.h"
#include"common.h"

cTimer::cTimer() {
    //clock_gettime(CLOCK_REALTIME, &process_start);
    //process_start = static_cast<float>(glfwGetTime());
    frame_start = process_start;
}

cTimer::~cTimer() {}

double cTimer::elapsed(bool frame) {
    //clock_gettime(CLOCK_REALTIME, &current);
    double elapsed =
        frame ? (current.tv_sec + current.tv_nsec / 1000000000.0 -
            frame_start.tv_sec - frame_start.tv_nsec / 1000000000.0)
        : (current.tv_sec + current.tv_nsec / 1000000000.0 -
            process_start.tv_sec - process_start.tv_nsec / 1000000000.0);
    frame_start = current;
    return elapsed;
}