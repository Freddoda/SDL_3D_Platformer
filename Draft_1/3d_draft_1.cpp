#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>

struct Point{
    float x;
    float y;
    float z;
};

struct Plane{
    //ax+by+cz+d=0
    float a;
    float b;
    float c;
    float d;
};

struct Line{
    Point point1;
    Point point2;
    //line is the intersection of the plains
    Plane plane1; //x&y
    Plane plane2; //x&z
    Plane plane3; //y&z

    Line(Point p1, Point p2){
        point1=p1;point2=p2;
        //calculate planes
    }
};

struct Polygon{
    Point point1;
    Point point2;
    Point point3;

    Plane plane;

    Polygon(Point p1, Point p2, Point p3){
        point1=p1;point2=p2;point3=p3;
        //calculate plane
    }
};

class Camera{
    private:
    float posX; float posY; float posZ;
    float angleA; //up&down ; -80,80
    float angleB; //left&right ; 0,360

    public:

    Camera(){}
    
    Camera(float posX, float posY, float posZ, float angleA, float angleB){
        this->posX=posX;this->posY=posY;this->posZ=posZ;this->angleA=angleA;this->angleB=angleB;
    }

    void update(const bool *keys, float mouseX, float mouseY, int windSize){
        angleA+=(mouseY-windSize/2);
        if (angleA<-80){
            angleA=-80;
        } else if (angleA>80){
            angleA=80;
        }
        angleB+=(mouseX-windSize/2);
        if (angleB<0){
            angleB+=360;
        } else if (angleB>360){
            angleB-=360;
        }


    }

    void render(SDL_GPUDevice* gpu, SDL_GPUCommandBuffer* commBuffer){

    }
};

class Cube{

};

class Program{
    private:
    int wSize;
    Camera camera;

    public:
    Program(int windowSize){
        wSize=windowSize;
        camera = Camera(0, 0, 0, 0, 0);
    }

    void update(const bool *keys, float mouseX, float mouseY, Uint32 mButtons, SDL_Window* window){
        camera.update(keys, mouseX, mouseY, wSize);
        SDL_WarpMouseInWindow(window,static_cast<float>(wSize)/2,static_cast<float>(wSize)/2);

    }

    void draw(SDL_GPUDevice* gpu){
        SDL_GPUCommandBuffer* commBuffer = SDL_AcquireGPUCommandBuffer(gpu);

        camera.render(gpu, commBuffer);

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

    window = SDL_CreateWindow("3D", windSize, windSize, SDL_WINDOW_MOUSE_FOCUS);
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


        program.update(keys, mouseX, mouseY, mButtons, window);
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