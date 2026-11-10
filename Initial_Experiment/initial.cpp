#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>
#include<vector>

class object{

};

class Camera{

};

class Program{
    private:
    int squareD, lineD;

    public:
    Program(int squareD, int lineD){
        this->squareD = squareD;
        this->lineD = lineD;
    }

    void update(const bool *keys, float &mouseX, float &mouseY){

    }

    void draw(SDL_Renderer *renderer){
        SDL_RenderClear(renderer);



        SDL_RenderPresent(renderer);
    }

};

int main(int argc, char* argv[]){
    SDL_Window* window;                    // Declare a window pointer
    SDL_Renderer* renderer;

    bool done = false;
    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL3

    const bool *keys = SDL_GetKeyboardState(NULL);
    float mouseX;
    float mouseY;
    Uint32 mButtons;

    std::chrono::nanoseconds start;
    std::chrono::nanoseconds end;
    std::chrono::nanoseconds duration;
    std::chrono::nanoseconds frametime(16666667);

    int squareD = 720;
    int lineD = 30;

    window = SDL_CreateWindow("2.5D", squareD, squareD + lineD, SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window,NULL);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    Program program(squareD, lineD);

    while (!done) {
        start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT){
                done = true;
            }
        }
        if (keys[SDL_SCANCODE_ESCAPE]){
            done = true;
        }
        mButtons = SDL_GetMouseState(&mouseX, &mouseY); //returns bitmask, use 'bitwise and' (&) and bitmask for intended Mbutton to check if clicked

        program.update(keys, mouseX, mouseY);
        program.draw(renderer);

        SDL_UpdateWindowSurface(window);

        end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
        duration = end - start;
        if (frametime>duration){
            std::this_thread::sleep_for(frametime-duration);
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);

    SDL_Quit();
    return 0;
}