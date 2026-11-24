#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>

struct Vect3d{
    float x;
    float y;
    float z;
};

struct Plane{
    //Ax+By+Cz=D
    float xcoeff;
    float ycoeff;
    float zcoeff;
    float constant;
};

struct Polygon{
    Plane plane;
    Vect3d vert1;
    Vect3d vert2;
    Vect3d vert3;

    Polygon(Vect3d vert1, Vect3d vert2, Vect3d vert3){
        this->vert1 = vert1;
        this->vert2 = vert2;
        this->vert3 = vert3;

        Vect3d dirVect12 = {vert1.x-vert2.x, vert1.y-vert2.y, vert1.z-vert2.z};
        Vect3d dirVect13 = {vert1.x-vert3.x, vert1.y-vert3.y, vert1.z-vert3.z};;
        Vect3d crossProd;
        crossProd.x = dirVect12.y*dirVect13.z-dirVect12.z*dirVect13.y;
        crossProd.y = dirVect12.z*dirVect13.x-dirVect12.x*dirVect13.z;
        crossProd.z = dirVect12.x*dirVect13.y-dirVect12.y*dirVect13.x;

        this->plane.xcoeff = crossProd.x;
        this->plane.ycoeff = crossProd.y;
        this->plane.zcoeff = crossProd.z;
        this->plane.constant = crossProd.x*vert1.x+crossProd.y*vert1.y+crossProd.z*vert1.z;
    }
};

struct Line{
    Vect3d posVect;
    Vect3d dirVect;
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