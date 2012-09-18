#include "SDL/SDL.h"

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
        bool isPaused();
        bool isStarted();

};

Timer::Timer(){
    startTicks = 0;
    pausedTicks = 0;
    started = false;
    paused = false;
}

void Timer::start(){
    started = true;
    paused = false;
    startTicks = SDL_GetTicks();
}

void Timer::stop(){
    started = false;
    paused = false;
    startTicks = 0;
    pausedTicks = 0;
}

void Timer::pause(){
    if (started && !paused){
        paused = true;
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause(){
    if (paused){
        paused = false;
        startTicks = SDL_GetTicks() - pausedTicks;
    }
}

bool Timer::isPaused(){
    return paused;
}

bool Timer::isStarted(){
    return started;
}

int Timer::getTicks(){
    if (started){
        if (paused)
            return pausedTicks;
        else
            return SDL_GetTicks() - startTicks;
    }
    return 0;
}




