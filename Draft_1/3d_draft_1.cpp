#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>
#include<chrono>
#include<thread>
#include<vector>
#include<array>
#include<cmath>
#include<iostream>

namespace Pi{
    const double pi = 3.14159265359;
}

struct Vect3d{
    float x;
    float y;
    float z;

    Vect3d  operator-(const Vect3d& v){
        return Vect3d{x-v.x,y-v.y,z-v.z};
    }
};

struct Plane{
    //Ax+By+Cz=D
    float xcoeff;
    float ycoeff;
    float zcoeff;
    float constant;
};

struct Line{
    Vect3d posVect;
    Vect3d dirVect;
};

struct Polygon{
    Plane plane;
    Line side1;
    Line side2;
    Line side3;

    Polygon(Vect3d vert1, Vect3d vert2, Vect3d vert3){
        this->side1.posVect = vert1;
        this->side1.dirVect = vert2-vert1;
        this->side2.posVect = vert2;
        this->side2.dirVect = vert3-vert2;
        this->side3.posVect = vert3;
        this->side3.dirVect = vert1-vert3;

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

struct Ray{
    Line rayLine;
    Vect3d collPoint;
    bool coll;
};

class Cube{
    private:
    Vect3d pos;
    Vect3d size;
    std::vector<Polygon> polygons;

    public:

    Cube(std::array<float,3> position, std::array<float,3> dimensions){
        pos.x=position[0]; pos.y=position[1]; pos.z=position[2];
        size.x=dimensions[0]; size.y=dimensions[1]; size.z=dimensions[2];

        //calculate the polygons
    }


    std::vector<Polygon> getPolygons(){
        return polygons;
    }

};

class Objectholder{
    private:
    std::vector<Cube> cubes;
    std::vector<Polygon> cubePolygons;
    std::vector<Polygon> polygons;

    public:
    Objectholder(){}

    void giveCubes(std::vector<Cube> cubes){
        this->cubes = cubes;

        for (Cube &cube : cubes){
            for (Polygon polygon : cube.getPolygons()){
                cubePolygons.push_back(polygon);
            }
        }
    }

    void givePolys(std::vector<Polygon> polys){
        this->polygons = polys;
    }

    void rayCollide(Ray* ray){
        std::vector<Vect3d> collPoints;
        std::vector<Polygon> polys;
        float coeff;
        bool inside = false;

        polys.insert(polys.end(),polygons.begin(),polygons.end());
        polys.insert(polys.end(),cubePolygons.begin(),cubePolygons.end());

        for (Polygon &poly : polys){
            if (poly.plane.xcoeff*ray->rayLine.dirVect.x + 
                poly.plane.ycoeff*ray->rayLine.dirVect.y + 
                poly.plane.zcoeff*ray->rayLine.dirVect.z == 0) {
                continue;
            }
            coeff = (poly.plane.constant - (poly.plane.xcoeff*ray->rayLine.posVect.x + 
                    poly.plane.ycoeff*ray->rayLine.posVect.y + 
                    poly.plane.zcoeff*ray->rayLine.posVect.z) )
                    / 
                    (poly.plane.xcoeff*ray->rayLine.dirVect.x + 
                    poly.plane.ycoeff*ray->rayLine.dirVect.y + 
                    poly.plane.zcoeff*ray->rayLine.dirVect.z);
            if (coeff>1 || coeff<0){
                continue;
            }

            inside = false;
            //project onto flat plane & check if in polygon
            for (Line &side : std::array<Line,3>{poly.side1,poly.side2,poly.side3}){
                //algebra has been simplified down, the checking vector is [1,0], its bounds are not checked, only the bounds on the polygon side are
                if (side.dirVect.y==0){
                    continue;
                }
                if (side.posVect.y-(ray->rayLine.posVect.y+coeff*ray->rayLine.dirVect.y)>1 || 
                    side.posVect.y-(ray->rayLine.posVect.y+coeff*ray->rayLine.dirVect.y)<0){
                    continue;
                }
                if (inside){
                    inside=false;
                    continue;
                }
                inside=true;
            }

            if (inside){
                collPoints.push_back(Vect3d{ray->rayLine.posVect.x+coeff*ray->rayLine.dirVect.x,
                                            ray->rayLine.posVect.y+coeff*ray->rayLine.dirVect.y,
                                            ray->rayLine.posVect.z+coeff*ray->rayLine.dirVect.z});
            }
        }

        if (collPoints.size()==0){
            return;
        }

        ray->coll=true;
        ray->collPoint=collPoints[0];
        for (Vect3d point : collPoints){
            if (point.x*point.x+point.y*point.y+point.z*point.z<
                ray->collPoint.x*ray->collPoint.x+ray->collPoint.y*ray->collPoint.y+ray->collPoint.z*ray->collPoint.z){
                ray->collPoint=point;
            }
        }
    }
};

class Camera{
    private:
    float posX; float posY; float posZ;
    float angleA; //up&down ; -80,80
    float angleB; //left&right ; 0,360
    float speed;
    float FOV;

    public:

    Camera(){}
    
    Camera(float posX, float posY, float posZ, float angleA, float angleB){
        this->posX=posX;this->posY=posY;this->posZ=posZ;this->angleA=angleA;this->angleB=angleB;
        speed = 1;
        FOV = 100;
    }

    void update(const bool *keys, float mouseX, float mouseY, int windSize){
        angleA-=(mouseY-windSize/2)/4;
        if (angleA<(-80.1)){
            angleA=-80;
        } else if (angleA>80){
            angleA=80;
        }
        angleB+=(mouseX-windSize/2)/4;
        if (angleB<0){
            angleB+=360;
        } else if (angleB>360){
            angleB-=360;
        }

        if (keys[SDL_SCANCODE_W]){
            posZ+= speed*std::sin(Pi::pi*(angleA/180.f));
            posX+= speed*std::cos(Pi::pi*(angleA/180.f))*std::cos(Pi::pi*(angleB/180.f));
            posY+= speed*std::cos(Pi::pi*(angleA/180.f))*std::sin(Pi::pi*(angleB/180.f));
        }
        if (keys[SDL_SCANCODE_S]){
            posZ-= speed*std::sin(Pi::pi*(angleA/180.f));
            posX-= speed*std::cos(Pi::pi*(angleA/180.f))*std::cos(Pi::pi*(angleB/180.f));
            posY-= speed*std::cos(Pi::pi*(angleA/180.f))*std::sin(Pi::pi*(angleB/180.f));
        }
        //add more movement options
    }

    void render(SDL_GPUDevice* gpu, SDL_GPUCommandBuffer* commBuffer, Objectholder &objectHolder, int wSize){
        std::vector<Ray> rays;
        float visDist = 600;

        for(float n1=0; n1<wSize; n1++){
            for(float n2=0; n2<wSize; n2++){
                rays.push_back(Ray{Line{Vect3d{posX,posY,posZ},
                    Vect3d{static_cast<float>(visDist*std::cos(Pi::pi*((angleA+(n1*FOV/wSize)-FOV/2)/180.f))*std::cos(Pi::pi*((angleB+(n2*FOV/wSize)-FOV/2)/180.f))), 
                        static_cast<float>(visDist*std::cos(Pi::pi*((angleA+(n1*FOV/wSize)-FOV/2)/180.f))*std::sin(Pi::pi*((angleB+(n2*FOV/wSize)-FOV/2)/180.f))), 
                        static_cast<float>(visDist*std::sin(Pi::pi*((angleA+(n1*FOV/wSize)-FOV/2)/180.f)))}}, NULL, false});
            }
        }

        for (Ray &ray : rays){
            objectHolder.rayCollide(&ray);
        }

        //draw
    }
};

class Program{
    private:
    int wSize;
    Camera camera;
    Objectholder objectHolder;

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

        camera.render(gpu, commBuffer, objectHolder, wSize);

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

    SDL_WarpMouseInWindow(window,static_cast<float>(windSize)/2,static_cast<float>(windSize)/2);

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