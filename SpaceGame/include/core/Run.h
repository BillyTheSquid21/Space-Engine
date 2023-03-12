#pragma once
#ifndef SG_RUN_H
#define SG_RUN_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "utility/SGUtil.h"
#include "core/Game.h"
#include "cstdlib"
#include "chrono"
#include "thread"
#include "core/Root.h"

namespace SG
{
	//Pointer to game kept outside for key callback benifit
	template<typename T>
	std::shared_ptr<T> game;

    const char SG_VERSION[] = "0.9.1" ;

    template<typename T>
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        game<T>->handleInput(key, scancode, action, mods);
    }

    template<typename T>
    void MouseCallback(GLFWwindow* window, int button, int action, int mods)
    {
        game<T>->handleMouse(button, action, mods);
    }

    template<typename T>
    void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
    {
        game<T>->handleScrolling(xOffset, yOffset);
    }

    template<typename T>
    void MousePosCallback(GLFWwindow* window, double xPos, double yPos)
    {
        game<T>->handleMousePos(xPos, yPos);
    }

	template<typename T>
	int Run(int width, int height, bool windowed)
    {
        using namespace std::chrono;
        static_assert(std::is_base_of<Game, T>());
        EngineLog("Space Engine Version ", SG_VERSION);
        //Init time - start and end, with delta time being the time between each run of the loop
        system_clock::time_point startTime = system_clock::now();
        system_clock::time_point endTime = system_clock::now();
        double deltaTime = 0.0f;

        //Heap allocates to new game, creates with resolution and name on tag - 
        //replace with derived game class
        {
            std::shared_ptr<T> gameInit(new T(width, height));
            game<T> = gameInit;
        }
        if (!game<T>->init("Space Engine", KeyCallback<T>, MouseCallback<T>, ScrollCallback<T>, MousePosCallback<T>, windowed)) {
            //If game fails to initialise program will not run
            return -1;
        }
        EngineLog("Game initialized successfully");

        //make delta time take average over 16 frames
        unsigned char deltaFrameCountLong = 1;
        unsigned char meanPeriodLong = 11;
        double deltaTimeLong = 0.0f;

        //make delta time take average over 6 frames
        unsigned char deltaFrameCountShort = 1;
        unsigned char meanPeriodShort = 7;
        double deltaTimeShort = 0.0f;

        //FPS Measure
        double previousTime = glfwGetTime();
        unsigned int framecount = 0;

        while (!glfwWindowShouldClose(game<T>->window) && SGRoot::CheckShouldRun())
        {
            //Measure speed
            double currentTime = glfwGetTime();
            game<T>->setTime(currentTime);
            framecount++;

            //If a second has passed.
            if (currentTime - previousTime >= 1.0)
            {
                SGRoot::FRAMERATE = framecount;

                framecount = 0;
                previousTime = currentTime;
            }

            //Takes time at start of loop
            startTime = system_clock::now();
            deltaFrameCountLong++;

            //Handle game processes
            game<T>->handleEvents();
            game<T>->update(deltaTime);
            game<T>->render();

            //Get time at end of loop
            endTime = system_clock::now();

            //Take time during work period
            duration<double, std::milli> workTime = endTime - startTime;

            //Check if program took less time to work than the cap
            if (workTime.count() < game<T>->secondsPerFrameCap())
            {
                //Works out time to sleep for by casting to double
                duration<double, std::milli> sleepDurationMS(game<T>->secondsPerFrameCap() - workTime.count());
                //Casts back to chrono type to get sleep time
                auto sleepDuration = duration_cast<milliseconds>(sleepDurationMS);
                //Sleeps this thread for calculated duration
                std::this_thread::sleep_for(milliseconds(sleepDuration.count()));
            }

            //Check to reset mean of frames
            if (deltaFrameCountLong == meanPeriodLong) {
                deltaFrameCountLong = 1;
                deltaTimeLong = 0.0f;
            }

            if (deltaFrameCountShort == meanPeriodShort) {
                deltaFrameCountShort = 1;
                deltaTimeShort = 0.0f;
            }

            //get time at end of all processes - time for one whole cycle
            endTime = system_clock::now();
            duration<double, std::milli> totalTime = endTime - startTime;
            deltaTimeLong = deltaTimeLong + (((totalTime).count() / 1000.0) - deltaTimeLong) / deltaFrameCountLong;
            deltaTimeShort = deltaTimeShort + (((totalTime).count() / 1000.0) - deltaTimeShort) / deltaFrameCountShort;
            deltaTime = (deltaTimeLong + deltaTimeShort) / 2.0f;
        }

        //clean and delete game
        game<T>->clean();
        game<T>.reset();

        return 0;
    }
}

#endif
