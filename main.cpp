#include <iostream>
#include "rayTracer.h"

int main()
{
    RayTracer rayTracer;
    rayTracer.width = 98;
    rayTracer.height = 48;
    rayTracer.thread_count = 3;
    rayTracer.skip_cout = false; // display only telemetry
    rayTracer.frame_count = 4500; // starting position (imo 4500 to 6000 looks great)

    rayTracer.camera.position = {3, 0, 0};
    rayTracer.camera.fov = 100;
    rayTracer.camera.up = {0,1,0};

    // add objects to scene
    rayTracer.scene.objects.push_back( new Sphere({0,0,0}, 1.5));
    rayTracer.scene.objects[0]->ANSI = "31;1"; // set to red and bold

    //rayTracer.scene.objects.push_back( new Sphere({0,0,0}, 1));
    rayTracer.scene.objects.push_back( new Ring({0,0,0}, {1,4,0}, 2, 3));


    // setup light source
    rayTracer.sun = {10, 5, 0};

    while (true){
        // interact with scene and camera
        rayTracer.camera.position.x = sin((double)rayTracer.frame_count / 750.0) * 4;
        rayTracer.camera.position.z = cos((double)rayTracer.frame_count / 750.0) * 4;
        rayTracer.camera.direction = rayTracer.camera.position.pointTo({0, 0, 0});

        // top text
        rayTracer.render();

        // telemetry
        cout << "\033[1mfps: " << 1000/(rayTracer.render_time);
        cout << " / render time: " << rayTracer.render_time << "ms\033[0m";
        cout << "\n frame: " << rayTracer.frame_count;
    }
    return 0;
}

