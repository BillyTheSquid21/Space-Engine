#include "core/Game.h"
#include <cstdlib>
#include <chrono>
#include <thread>

//Engine version tag
const char SG_VERSION[]{ "0.5" };

//saves space
using namespace std::chrono;

//Pointer to game kept outside for key callback benifit - will auto destroy
std::shared_ptr<Game> game;

int main(void) //TODO: get delta time working perfectly
{   
    EngineLog("Space Engine Version", SG_VERSION);
    //Init time - start and end, with delta time being the time between each run of the loop
    system_clock::time_point startTime = system_clock::now();
    system_clock::time_point endTime = system_clock::now();
    double deltaTime = 0.0f;

    //Heap allocates to new game, creates with resolution and name on tag - 
    //replace with derived game class
    {
        std::shared_ptr<Game> gameInit(new Game(640, 640));
        game = gameInit;
    }
    if (!game->init("Space Game", KeyCallback, MouseCallback, ScrollCallback)) {
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
  
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(game->window) && !Game::s_Close)
    {  
        // Measure speed
        double currentTime = glfwGetTime();
        game->setTime(currentTime);
        framecount++;

        // If a second has passed.
        if (currentTime - previousTime >= 1.0)
        {
            // Display the frame count here any way you want.
            EngineLog("FPS: ", framecount);
            EngineLog(deltaTime);

            framecount = 0;
            previousTime = currentTime;
        }

        //Takes time at start of loop
        startTime = system_clock::now();
        deltaFrameCountLong++;

        //Handle game processes
        game->handleEvents();
        game->update(deltaTime);
        game->render();

        //Get time at end of loop
        endTime = system_clock::now();

        //Take time during work period
        duration<double, std::milli> workTime = endTime - startTime;

        //Check if program took less time to work than the cap
        if (workTime.count() < game->secondsPerFrameCap())
        {
            //Works out time to sleep for by casting to double
            duration<double, std::milli> sleepDurationMS(game->secondsPerFrameCap() - workTime.count());
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
    game->clean();
    game.reset();

    return 0;
}

//deals with key callback
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    game->handleInput(key, scancode, action, mods);
}

void MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    game->handleMouse(button, action, mods);
}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    game->handleScrolling(xOffset, yOffset);
}

