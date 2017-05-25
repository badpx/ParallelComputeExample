//
// Created by dk on 17-5-25.
//

#ifndef PARALLELCOMPUTEEXAMPLE_TIMER_H
#define PARALLELCOMPUTEEXAMPLE_TIMER_H

#include <sys/time.h>
class Timer {
public:
    static long currentTimeMillis() {
        timeval time;
        gettimeofday( &time, NULL );
        return (1000000 * time.tv_sec + time.tv_usec) / 1000;   // us to ms
    }

    void start() {
        gettimeofday( &mStart, NULL );
        mLast = mStart;
    }

    long deltaMetering() {
        timeval time;
        gettimeofday( &time, NULL );
        long delta = (1000000 * (time.tv_sec - mLast.tv_sec ) + time.tv_usec - mLast.tv_usec) / 1000;
        mLast = time;
        return delta;
    }

    long duration() {
        timeval time;
        gettimeofday( &time, NULL );
        return (1000000 * (time.tv_sec - mStart.tv_sec ) + time.tv_usec - mStart.tv_usec) / 1000;
    }

    long reset() {
        long total = duration();
        start();
        return total;
    }

private:
    timeval mStart;
    timeval mLast;
};
#endif //PARALLELCOMPUTEEXAMPLE_TIMER_H
