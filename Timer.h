#ifndef GUARD_timer_h
#define GUARD_timer_h

class Timer{
    private:
    int startTicks;
    int pausedTicks;
    bool started;
    bool paused;

    public:
    Timer();
    void start();
    void stop();
    void pause();
    void unpause();
    int getTicks();
    bool isStarted();
    bool isPaused();

};

#endif
