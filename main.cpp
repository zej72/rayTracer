#include <iostream>
#include "rayTracer.h"

int main()
{
    RayTracer rayTracer;
    rayTracer.width = 98;
    rayTracer.height = 50;
    rayTracer.skip_cout = false;

    rayTracer.camera.position = {0, 1.7, 0};
    rayTracer.camera.fov = 100;

    // add objects to scene
    rayTracer.scene.objects.push_back( new Sphere({0,2,0}, 2));
    //rayTracer.scene.objects.push_back( new Plane({0,0,0},{0,1,0}));

    // setup light source
    rayTracer.sun = {1.5,1,0};
    rayTracer.sun.normalize();

    while (true){
        // interact with scene and camera
        rayTracer.camera.position.z = sin((double)rayTracer.frame_count / 300.0) * 4;
        rayTracer.camera.position.x = cos((double)rayTracer.frame_count / 300.0) * 4;

        //rayTracer.scene.objects[0]->size = rayTracer.scene.objects[0]->position.y = sin((double)rayTracer.frame_count / 100.0) + 1;

        rayTracer.camera.direction = rayTracer.camera.position.pointTo({0, 1.7, 0});

        // render frame
        rayTracer.render();

        // telemetry
        cout << "\033[1mtotal fps: " << 1000/(rayTracer.render_time);
        cout << " / total render time: " << rayTracer.render_time << "ms\033[0m";
    }
    return 0;
}

