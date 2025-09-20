#include "rayTracer.h"
#include "pixel_values.cpp"
extern unordered_map<int, string> ext_pixel_value;

using namespace std;

RayTracer::RayTracer(){
    this->frame = "";
    this->width = 98;
    this->height = 50;
    this->pixel_values = ext_pixel_value;
    this->skip_cout = false;
    this->frame_count = 0;
    this->thread_count = 1;

    this->camera.position = {0, 1.7, 0};
    this->camera.yaw = 1;
    this->camera.pitch = 0;
    this->camera.updateDirection();
    this->camera.fov = 100;
    system("clear");
}

void RayTracer::bufferDraw(int value){
    //int max_value = this->pixel_values.size();
    string pixel = this->pixel_values[clamp(value, 0, 4)];
    this->frame += pixel;
}

void RayTracer::bufferNextLine(){
    this->frame += "\n";
}

void RayTracer::bufferClear(){
    this->frame = "";
}

void RayTracer::screenClear(){
    system("clear");
}

void RayTracer::moveCursor(int x, int y) {
    cout << "\033[" << y << ";" << x << "H";
}

void RayTracer::present(){

    this->moveCursor(1, 1);

    if (skip_cout){return;}
    cout << this->frame;
    //cout.flush();
}

void RayTracer::render(){
    this->bufferClear();

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    std::vector<std::future<string>> futures; // hold threads

    // Create threads
    for (int i = 0; i < this->thread_count; ++i) {
        float step = this->height/this->thread_count;
        int start = (int)round(step * i);
        int end = (int)round(step * (i+1));

        futures.push_back(std::async(std::launch::async, [this, start, end]() { return this->main(start, end); }));
    }


    // Retrieve results from threads
    for (int i = 0; i < this->thread_count; ++i) {
        this->frame += futures[i].get();
    }

    this->present();
    this->frame_count++;

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    this->render_time = chrono::duration_cast<chrono::nanoseconds> (end - begin).count() / 1000000.0;
}

string RayTracer::main(int start, int end) {
  bool ray_collided;
  string buffer;
  string last_ANSI;
  Render_data data;

  for (int y = start; y < end; ++y) {
    for (int x = 0; x < this->width * 2; ++x) {
      // main render code. edit to your liking C:
      data.color = 0;
      data.ANSI = "";

      data.ray =
          this->camera.getRay((float)x / 2.0f, y, this->width, this->height);

      data.shadow_rendering = false;
      ray_collided = this->scene.intersect(data);
      if (ray_collided) {
        // dither effect
        data.color = (x % 4) + 4;

        data.color =
            (int)round((float)data.color *
                       cos(data.normal.angle(data.point.pointTo(this->sun))));

        // ray trace shadows
        data.shadow_rendering = true;
        data.ray = {data.point, data.point.pointTo(this->sun)};
        ray_collided = this->scene.intersect(data);
        ray_collided = data.ray.origin.distance(this->sun) > data.distance;
        if (ray_collided && false) { // remove "&& false" for black shadows
          data.color = 0;
        } else if (ray_collided && data.ANSI == "") {
          data.ANSI = "2";
        } else if (ray_collided) {
          data.ANSI += ";2";
        }
      }
      if (data.ANSI != last_ANSI) {
        buffer += "\033[0m";
      }
      if (data.ANSI != "") {
        buffer += "\033[" + data.ANSI + "m";
      }
      last_ANSI = data.ANSI;
      buffer += this->pixel_values[clamp(data.color, 0, 4)];
    }
    buffer += "\n";
  }
  buffer += "\033[0m";
  return buffer;
}
