#include <iostream>
#include "rayTracer.h"

int main()
{
    RayTracer rayTracer;
    rayTracer.width = 98;
    rayTracer.height = 50;
    rayTracer.thread_count = 3;
    rayTracer.skip_cout = false; // display only telemetry

    rayTracer.camera.position = {0, 0, 0};
    rayTracer.camera.fov = 100;
    rayTracer.camera.up = {0,1,0};

    // add objects to scene
    rayTracer.scene.objects.push_back( new Sphere({0,0,0}, 1.5));
    rayTracer.scene.objects.push_back( new Sphere({0,0,0}, 1));
    //rayTracer.scene.objects.push_back( new Plane({0,0,0},{0,1,0}));

    // setup light source
    rayTracer.sun = {2, 3, 0};
    rayTracer.sun.normalize();

    while (true){
        // interact with scene and camera
        rayTracer.camera.position.x = sin((double)rayTracer.frame_count / 750.0) * 4;
        rayTracer.camera.position.z = cos((double)rayTracer.frame_count / 750.0) * 4;
        rayTracer.scene.objects[0]->position.y = cos((double)rayTracer.frame_count / 300.0 + 3.1415) * 1;
        rayTracer.scene.objects[0]->position.x = sin((double)rayTracer.frame_count / 300.0 + 3.1415) * 1;
        rayTracer.scene.objects[1]->position.y = cos((double)rayTracer.frame_count / 300.0) * 2.5;
        rayTracer.scene.objects[1]->position.x = sin((double)rayTracer.frame_count / 300.0) * 2.5;

        rayTracer.camera.direction = rayTracer.camera.position.pointTo({0, 0, 0});

        // render frame
        rayTracer.render();

        // telemetry
        cout << "\033[1mtotal fps: " << 1000/(rayTracer.render_time);
        cout << " / render time: " << rayTracer.render_time << "ms\033[0m";
        cout << "\ncamera position: " << rayTracer.camera.position.x << ", " << rayTracer.camera.position.y << ", " << rayTracer.camera.position.z << "        ";
    }
    return 0;
}

