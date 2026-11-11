#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>
#include<vector>
#include<array>
#include<iostream>
#include<cmath>

namespace Pi{
    const double pi = 3.14159265359;
}

class Object1{
    //rectangle
    private:
    int posX, posY;
    int sizeX, sizeY;
    
    public:
    Object1(int posX, int posY, int sizeX, int sizeY){
        this->posX = posX;
        this->posY = posY;
        this->sizeX = sizeX;
        this->sizeY = sizeY;
    }

    void draw(SDL_Renderer* renderer, std::array<float,2> rootPos, int squareD){
        SDL_SetRenderDrawColor(renderer, 128, 128, 255, 255);
        SDL_FRect rect = {posX+squareD/2-rootPos[0], posY+squareD/2-rootPos[1], static_cast<float>(sizeX), static_cast<float>(sizeY)};
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Object2{
    //polygon
    private:
    std::vector<std::array<int,2>> vertices;

    public:
    Object2(std::vector<std::array<int,2>> vertices){
        this->vertices = vertices;
    }

    void draw(SDL_Renderer* renderer, std::array<float,2> rootPos, int squareD){
        if (vertices.size()<=2){
            return;
        }
        SDL_Vertex V[] = {
            {{static_cast<float>(vertices[0][0]-rootPos[0]+squareD/2),static_cast<float>(vertices[0][1]-rootPos[1]+squareD/2)}, {0.5,0.5,1}},
            {{static_cast<float>(vertices[1][0]),static_cast<float>(vertices[1][1])}, {0.5,0.5,1}},
            {{static_cast<float>(vertices[2][0]),static_cast<float>(vertices[2][1])}, {0.5,0.5,1}}
        };
        for (int i=2; i<vertices.size(); i++){
            V[1].position.x=vertices[i-1][0]-rootPos[0]+squareD/2;
            V[1].position.y=vertices[i-1][1]-rootPos[1]+squareD/2;
            V[2].position.x=vertices[i][0]-rootPos[0]+squareD/2;
            V[2].position.y=vertices[i][1]-rootPos[1]+squareD/2;
            SDL_RenderGeometry(renderer, NULL, V, 3, NULL, 0);
        }
    }

};

class Objectholder{
    private:
    std::vector<Object1> Obj1;
    std::vector<Object2> Obj2;

    public:
    Objectholder(std::vector<Object1> Obj1, std::vector<Object2> Obj2){
        this->Obj1 = Obj1;
        this->Obj2 = Obj2;
    }

    Objectholder(std::vector<Object1> Obj1){
        this->Obj1 = Obj1;
        this->Obj2 = std::vector<Object2>{};
    }

    Objectholder(){}

    void draw(SDL_Renderer* renderer, std::array<float,2> rootPos, int squareD){
        for (Object1 Obj: Obj1){
            Obj.draw(renderer, rootPos, squareD);
        }
        for (Object2 Obj: Obj2){
            Obj.draw(renderer, rootPos, squareD);
        }
    }

    bool collision(std::array<float,2> start, std::array<float,2> end){
        //do collision detection

        return false;
    }
};

struct Ray{
    float posX;
    float posY;
    int angle;
    int colldist;
    bool coll;
};

class Camera{
    private:
    float posX, posY;
    int angle, FOV;

    public:
    Camera(){
        *this = Camera(0,0,0,0);
    }

    Camera(float posX, float posY, int angle, int FOV){
        this->posX = posX;
        this->posY = posY;
        this->angle = angle;
        this->FOV = FOV;
    }

    std::array<float,2> getPos(){
        return std::array<float,2>{posX,posY};
    }

    void move(const bool *keys, Objectholder objs){
        if (keys[SDL_SCANCODE_D]){
            angle-=1;
            if (angle<0){
                angle+=360;
            }
        }
        if (keys[SDL_SCANCODE_A]){
            angle+=1;
            if (angle>360){
                angle-=360;
            }
        }
        //add collision later
        if (keys[SDL_SCANCODE_W]){
            posX+=sin(angle/180.f*Pi::pi);
            posY+=cos(angle/180.f*Pi::pi);
        }
        if (keys[SDL_SCANCODE_S]){
            posX-=sin(angle/180.f*Pi::pi);
            posY-=cos(angle/180.f*Pi::pi);
        }
    }

    void draw(SDL_Renderer* renderer, int squareD){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int x=0; x<=10; x++){
            for (int y=0; y<=10; y++){
                if ((x-5)*(x-5)+(y-5)*(y-5)<=25){
                    SDL_RenderPoint(renderer, static_cast<float>(squareD/2+x), static_cast<float>(squareD/2+y));
                }
            }
        }
    }

    void render(SDL_Renderer*, Objectholder objs, int squareD, int lineD){
        std::vector<Ray> rays;
        for (int i=0; i<=FOV; i++){
            rays.push_back(Ray{posX, posY, angle-FOV/2+i, 0, false});
        }
        //do rendering
    }
};

class Program{
    private:
    int squareD, lineD;
    Camera camera;
    Objectholder objs;

    public:
    Program(int squareD, int lineD){
        this->squareD = squareD;
        this->lineD = lineD;
        camera = Camera(0,0,0,100);
        objs = Objectholder(std::vector<Object1>{Object1(40,50,20,30)}, 
            std::vector<Object2>{Object2(std::vector<std::array<int,2>>{
                std::array<int,2>{-40,-60},std::array<int,2>{-50,-40},std::array<int,2>{0,-20},std::array<int,2>{-30,-70}})}); //put some stuff in
    }

    void update(const bool *keys, float &mouseX, float &mouseY){
        camera.move(keys, objs);
    }

    void draw(SDL_Renderer *renderer){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        camera.draw(renderer,squareD);
        objs.draw(renderer, camera.getPos(), squareD);

        camera.render(renderer, objs, squareD, lineD);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderLine(renderer, static_cast<float>(0), static_cast<float>(squareD),static_cast<float>(squareD),static_cast<float>(squareD));
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
    int lineD = 80;

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