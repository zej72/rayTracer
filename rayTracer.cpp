#include "rayTracer.h"
#include "pixel_values.cpp"
extern unordered_map<int, string> ext_pixel_value;

using namespace std;



Vec3 Vec3::operator+(const Vec3& other) const {
    return {x + other.x, y + other.y, z + other.z};
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return {x - other.x, y - other.y, z - other.z};
}

Vec3 Vec3::operator*(float scalar) const {
    return {x * scalar, y * scalar, z * scalar};
}

Vec3 Vec3::normalize() const {
    float length = sqrt(x * x + y * y + z * z);
    return {x / length, y / length, z / length};
}

float Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

float Vec3::distance(const Vec3& other) const {
    return sqrt(
        pow(x - other.x, 2) +
        pow(y - other.y, 2) +
        pow(z - other.z, 2)
        );
}

Vec3 Vec3::pointTo(const Vec3& other) const {
    Vec3 direction = other - Vec3 {x, y, z};
    return direction.normalize();
}

float Vec3::angle(const Vec3& other) const {
    float dot = x*other.x + y*other.y + z*other.z;
    float lenSqThis = pow(x,2) + pow(y,2) + pow(z,2);
    float lenSqOther = pow(other.x, 2) + pow(other.y, 2) + pow(other.z, 2);
    return acos(dot/sqrt(lenSqThis * lenSqOther));
}



void Camera::rotate(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch += deltaPitch;

    // Clamp pitch to avoid gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateDirection();
}

void Camera::updateDirection() {
    float radYaw = yaw * (M_PI / 180.0f);
    float radPitch = pitch * (M_PI / 180.0f);

    direction.x = cos(radYaw) * cos(radPitch);
    direction.y = sin(radPitch);
    direction.z = sin(radYaw) * cos(radPitch);
    direction = direction.normalize();
}

Ray Camera::getRay(float x, float y, int width, int height) {
    float aspectRatio = (float)width / (float)height;
    float scale = tan(fov * 0.5f * (M_PI / 180.0f));

    // Calculate the ray direction based on the camera's orientation
    Vec3 right = direction.cross(this->up).normalize(); // Right vector
    Vec3 up = right.cross(direction).normalize();

    // Calculate the pixel position in normalized device coordinates
    float pixelX = (2 * (x + 0.25f) / width - 1) * aspectRatio * scale;
    float pixelY = (1 - 2 * (y + 0.5f) / height) * scale;

    // Calculate the ray direction
    Vec3 rayDir = (direction + right * pixelX + up * pixelY).normalize();
    return {position, rayDir};
}



Plane::Plane(Vec3 p, Vec3 n, float s){
    position = p;
    direction = n;
    size = s;
    cast_shadow = true;
}

bool Plane::intersect(const Ray ray, float& t, Vec3& n) const{
    float denominator = ray.direction.dot(direction);

    // If the denominator is close to zero, the ray is parallel to the plane
    if (fabs(denominator) < 1e-6) {
        return false; // No intersection
    }

    Vec3 difference = position - ray.origin;
    t = difference.dot(direction) / denominator;

    // If t is negative, the intersection point is behind the ray's origin
    if (t < 0.001f) {
        return false;
    }
    Vec3 intersection_point = ray.origin + ray.direction * t;
    if (intersection_point.distance(position) > size){
        return false;
    }

    n = direction;

    if (difference.dot(direction) > 0) {
        n = {-direction.x, -direction.y, -direction. z};
    }

    return true;
}


Sphere::Sphere(Vec3 c, float r){
    position = c;
    size = r;
    cast_shadow = true;
}

bool Sphere::intersect(const Ray ray, float& t, Vec3& n) const{
    Vec3 oc = ray.origin - position;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - pow(size, 2);
    float discriminant = b * b - 4 * a * c;
    Vec3 collision_point;

    if (discriminant < 0) {
        return false; // No intersection
    } else {
        float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

        // closest positive t
        t = (t1 > 0) ? t1 : t2;

        if (t < 0.01f) {
            return false;
        }

        collision_point = ray.origin + ray.direction * t;
        n = collision_point - position;
        n.normalize();

        return true;
    }
}


Ring::Ring(Vec3 c, Vec3 n, float in_s, float out_s){
    position = c;
    direction = n;
    size = in_s;
    size2 = out_s;
}

bool Ring::intersect(const Ray ray, float& t, Vec3& n) const{
    float denominator = ray.direction.dot(direction);

    // If the denominator is close to zero, the ray is parallel to the plane
    if (fabs(denominator) < 1e-6) {
        return false; // No intersection
    }

    Vec3 difference = position - ray.origin;
    t = difference.dot(direction) / denominator;

    // If t is negative, the intersection point is behind the ray's origin
    if (t < 0.001f) {
        return false;
    }
    Vec3 intersection_point = ray.origin + ray.direction * t;
    if (size > intersection_point.distance(position) || size2 < intersection_point.distance(position)){
        return false;
    }

    n = direction;

    if (difference.dot(direction) > 0) {
        n = {-direction.x, -direction.y, -direction. z};
    }


    return true;
}


bool Scene::intersect(Render_data &data) const {
  float closest_t = std::numeric_limits<float>::max();
  bool hit = false;
  float t;
  Vec3 n;

  for (const auto &obj : objects) {
    if (obj->intersect(data.ray, t, n) && t < closest_t &&
        !(data.shadow_rendering && !obj->cast_shadow)) {
      closest_t = t;
      hit = true;
      data.point = data.ray.origin + data.ray.direction * closest_t;
      data.normal = n;
      data.ANSI = obj->ANSI;
      data.distance = closest_t;
    }
  }
  return hit;
}


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
