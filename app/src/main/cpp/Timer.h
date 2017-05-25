//
// Created by dk on 17-5-25.
//

#ifndef PARALLELCOMPUTEEXAMPLE_TIMER_H
#define PARALLELCOMPUTEEXAMPLE_TIMER_H

#include <sys/time.h>
class Timer {
public:
    static uint32_t currentTimeMillis() {
        timeval time;
        gettimeofday( &time, NULL );
        return (uint32_t) (1000000 * time.tv_sec + time.tv_usec);
    }

    void start() {
        gettimeofday( &mStart, NULL );
        mLast = mStart;
    }

    uint32_t deltaMetering() {
        timeval time;
        gettimeofday( &time, NULL );
        uint32_t delta = (uint32_t) (1000000 * (time.tv_sec - mStart.tv_sec ) + time.tv_usec - mStart.tv_usec);
        mLast = time;
        return delta;
    }

    uint32_t duration() {
        timeval time;
        gettimeofday( &time, NULL );
        return (uint32_t) (1000000 * (time.tv_sec - mStart.tv_sec ) + time.tv_usec - mStart.tv_usec);
    }

    uint32_t stop() {
        return duration();
    }

private:
    timeval mStart;
    timeval mLast;
};
#endif //PARALLELCOMPUTEEXAMPLE_TIMER_H
