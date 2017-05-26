package com.tencent.parallelcomputedemo;

import android.os.SystemClock;

/**
 * Created by dk on 17-5-26.
 */

public class Timer {
    public static long uptimeMillis() {
        return SystemClock.uptimeMillis();
    }

    public void start() {
        mLast = mStart = SystemClock.uptimeMillis();
    }

    public long deltaMetering() {
        long time = SystemClock.uptimeMillis();
        long delta = time - mLast;
        mLast = time;
        return delta;
    }

    public long duration() {
        return SystemClock.uptimeMillis()- mStart;
    }

    public long reset() {
        long total = duration();
        start();
        return total;
    }

    private long mStart;
    private long mLast;
}
