#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>

class Program{
    private:
    int wSize;

    public:
    Program(int windowSize){
        wSize=windowSize;
    }

    void update(const bool *keys, float mouseX, float mouseY, Uint32 mButtons){

    }

    void draw(SDL_GPUDevice* gpu){
        SDL_GPUCommandBuffer* commBuffer = SDL_AcquireGPUCommandBuffer(gpu);



        SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commBuffer);
        SDL_WaitForGPUFences(gpu, true, &fence, 1);
        SDL_ReleaseGPUFence(gpu, fence);
        return;
    }
};


int main(int argc, char* argv[]){
    SDL_GPUDevice* gpu;
    SDL_Window* window; 
    
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

    int windSize = 720;
    Program program = Program(windSize);

    window = SDL_CreateWindow("3D", windSize, windSize, NULL);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }
    gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL, true, NULL);
    if (gpu == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create gpu: %s\n", SDL_GetError());
        return 2;
    }
    SDL_ClaimWindowForGPUDevice(gpu, window);

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
        mButtons = SDL_GetMouseState(&mouseX, &mouseY);


        program.update(keys, mouseX, mouseY, mButtons);
        program.draw(gpu);


        end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
        duration = end - start;
        if (frametime>duration){
            std::this_thread::sleep_for(frametime-duration);
        }
    }


    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(gpu);
    SDL_Quit();
    return 0;
}