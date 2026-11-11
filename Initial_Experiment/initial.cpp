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

struct line{
    float x1;
    float y1;
    float x2;
    float y2;
};

std::array<float,3> collide(line line1, line line2){
    //first number functions as a bool
    float commonX;
    float commonY;
    float g1; float c1;
    float g2; float c2;
    bool l1vert = false;
    if (line1.x1==line1.x2){
        l1vert = true;
        commonX=line1.x1;
    } else {
        g1 = (line1.y1-line1.y2)/(line1.x1-line1.x2);
        c1 = line1.y1-line1.x1*g1;
    }
    bool l2vert = false;
    if (line2.x1==line2.x2){
        l2vert = true;
        commonX=line2.x1;
    } else {
        g2 = (line2.y1-line2.y2)/(line2.x1-line2.x2);
        c2 = line2.y1-line2.x1*g2;
    }
    if (l1vert&&l2vert){
        return std::array<float,3>{0,0,0};
    }
    if (!(l1vert||l2vert)){
        if (g1==g2){
            return std::array<float,3>{0,0,0};
        }
        commonX = (c2-c1)/(g1-g2);
    }
    if (!l1vert){
        commonY = commonX*g1+c1;
    } else {
        commonY = commonX*g2+c2;
    }

    if (
        (commonX>=line1.x1&&commonX<=line1.x2) || (commonX<=line1.x1&&commonX>=line1.x2)
    ){
        if (
            (commonX>=line2.x1&&commonX<=line2.x2) || (commonX<=line2.x1&&commonX>=line2.x2)
        ){
            if (
                (commonY>=line1.y1&&commonY<=line1.y2) || (commonY<=line1.y1&&commonY>=line1.y2)
            ){
                if (
                    (commonY>=line2.y1&&commonY<=line2.y2) || (commonY<=line2.y1&&commonY>=line2.y2)
                ){
                    return std::array<float,3>{1,commonX,commonY};
                }
            }
        }
    }
    return std::array<float,3>{0,commonX,commonY};
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

    std::vector<line> getLines(){
        return std::vector<line>{
            line{static_cast<float>(posX),static_cast<float>(posY),static_cast<float>(posX+sizeX),static_cast<float>(posY)},
            line{static_cast<float>(posX),static_cast<float>(posY),static_cast<float>(posX),static_cast<float>(posY+sizeY)},
            line{static_cast<float>(posX+sizeX),static_cast<float>(posY),static_cast<float>(posX+sizeX),static_cast<float>(posY+sizeY)},
            line{static_cast<float>(posX),static_cast<float>(posY+sizeY),static_cast<float>(posX+sizeX),static_cast<float>(posY+sizeY)}
        };
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

    std::vector<line> getLines(){
        std::vector<line> lines;
        for (int i=1; i<vertices.size(); i++){
            lines.push_back(line{static_cast<float>(vertices[i-1][0]),static_cast<float>(vertices[i-1][1]),
                static_cast<float>(vertices[i][0]),static_cast<float>(vertices[i][1])});
        }
        lines.push_back(line{static_cast<float>(vertices[vertices.size()-1][0]),static_cast<float>(vertices[vertices.size()-1][1]),
            static_cast<float>(vertices[0][0]),static_cast<float>(vertices[0][1])});
        return lines;
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

    std::vector<std::array<float,2>> collision(line line1){
        std::vector<std::array<float,2>> collPoints;
        std::vector<line> lines;
        std::array<float,3> cache;

        for (Object1 obj : Obj1){
            for (line L: obj.getLines()){
                lines.push_back(L);
            }
        }
        for (Object2 obj : Obj2){
            for (line L: obj.getLines()){
                lines.push_back(L);
            }
        }

        for (line L : lines){
            cache = collide(line1, L);
            if (cache[0]==1){
                collPoints.push_back(std::array<float,2>{cache[1],cache[2]});
            }
        }

        return collPoints;
    }
};

struct Ray{
    float posX;
    float posY;
    float angle;
    float colldist;
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
            if (objs.collision(line{posX,posY,posX+static_cast<float>(sin(angle/180.f*Pi::pi)),posY}).size()==0){
                posX+=sin(angle/180.f*Pi::pi);
            }
            if (objs.collision(line{posX,posY,posX,posY+static_cast<float>(cos(angle/180.f*Pi::pi))}).size()==0){
                posY+=cos(angle/180.f*Pi::pi);
            }
        }
        if (keys[SDL_SCANCODE_S]){
            if (objs.collision(line{posX,posY,posX-static_cast<float>(sin(angle/180.f*Pi::pi)),posY}).size()==0){
                posX-=sin(angle/180.f*Pi::pi);
            }
            if (objs.collision(line{posX,posY,posX,posY-static_cast<float>(cos(angle/180.f*Pi::pi))}).size()==0){
                posY-=cos(angle/180.f*Pi::pi);
            }
        }
    }

    void draw(SDL_Renderer* renderer, int squareD){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int x=0; x<=10; x++){
            for (int y=0; y<=10; y++){
                if ((x-5)*(x-5)+(y-5)*(y-5)<=25){
                    SDL_RenderPoint(renderer, static_cast<float>(squareD/2+x-5), static_cast<float>(squareD/2+y-5));
                }
            }
        }
    }

    void render(SDL_Renderer*, Objectholder objs, int squareD, int lineD){
        std::vector<Ray> rays;
        for (int i=0; i<=FOV; i++){
            rays.push_back(Ray{posX, posY, static_cast<float>(angle-FOV/2+i), 0.f, false});
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