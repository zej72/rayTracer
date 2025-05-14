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
    Vec3 right = {direction.z, 0, -direction.x}; // Right vector
    Vec3 up = {0, 1, 0}; // Up vector (world up)

    // Calculate the pixel position in normalized device coordinates
    float pixelX = (2 * (x + 0.25f) / width - 1) * aspectRatio * scale;
    float pixelY = (1 - 2 * (y + 0.5f) / height) * scale;

    // Calculate the ray direction
    Vec3 rayDir = (direction + right * pixelX + up * pixelY).normalize();
    return {position, rayDir};
}



Plane::Plane(Vec3 p, Vec3 n){
    position = p;
    direction = n;
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

    n = direction;

    return true;
}


Sphere::Sphere(Vec3 c, float r){
    position = c;
    size = r;
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


bool Scene::intersect(const Ray& ray, float& closestT, Vec3& intersection_position, Vec3& normal) const {
    closestT = std::numeric_limits<float>::max();
    bool hit = false;

    for (const auto& obj : objects) {
        float t;
        Vec3 n;
        if (obj->intersect(ray, t, n) && t < closestT) {
            closestT = t;
            hit = true;
            intersection_position = ray.origin + ray.direction * closestT;
            normal = n;
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
    int color;
    Vec3 intersection_point;
    Vec3 intersection_point_offset_direction;
    Vec3 normal;
    Ray ray;
    bool ray_collided;
    float distance;

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    this->bufferClear();

    for(int y = 0 ; y < this->height; ++y) {
        for(int x = 0; x < this->width*2; ++x) {
            // main render code. edit to your liking C:
            color = 0;

            ray = this->camera.getRay((float)x/2.0f, y, this->width, this->height);

            ray_collided = this->scene.intersect(ray, distance, intersection_point, normal);
            if (ray_collided){
                color = 4;

                color = (int)round((float) color * cos(normal.angle(sun))); // Lambert's cosine law
                intersection_point_offset_direction = intersection_point + this->sun * 0.05f;                
                ray = {intersection_point_offset_direction, this->sun};
                ray_collided = this->scene.intersect(ray, distance, intersection_point, normal);
                if (ray_collided){
                    color = 0;
                }
            }
            this->bufferDraw(color);
        }
        this->bufferNextLine();
    }

    this->present();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    this->render_time = chrono::duration_cast<chrono::nanoseconds> (end - begin).count() / 1000000.0;
    this->frame_count++;
}
