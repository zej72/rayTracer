#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <optional>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

#include "pixel_values.cpp"
extern unordered_map<int, string> ext_pixel_value;

struct Vec3 {
    float x, y, z;

    Vec3 operator+(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator*(float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    Vec3 normalize() const {
        float length = sqrt(x * x + y * y + z * z);
        return {x / length, y / length, z / length};
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    float distance(const Vec3& other) const {
        return sqrt(
            pow(x - other.x, 2) +
            pow(y - other.y, 2) +
            pow(z - other.z, 2));
    }

    Vec3 pointTo(const Vec3& other) const {
        Vec3 direction = other - Vec3 {x, y, z};
        return direction.normalize();
    }
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

struct Camera {
    Vec3 position;
    Vec3 direction;
    Vec3 up;
    float fov;
    float yaw;
    float pitch;

    void rotate(float deltaYaw, float deltaPitch) {
        yaw += deltaYaw;
        pitch += deltaPitch;

        // Clamp pitch to avoid gimbal lock
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateDirection();
    }

    void updateDirection() {
        float radYaw = yaw * (M_PI / 180.0f);
        float radPitch = pitch * (M_PI / 180.0f);

        direction.x = cos(radYaw) * cos(radPitch);
        direction.y = sin(radPitch);
        direction.z = sin(radYaw) * cos(radPitch);
        direction = direction.normalize();
    }

    Ray getRay(float x, float y, int width, int height) {
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
};

class SceneObject {
public:
    string name;
    Vec3 position;
    Vec3 direction;
    float size;
    virtual bool intersect(const Ray ray, float& t) const = 0;
    virtual ~SceneObject(){}
};

class Plane : public virtual SceneObject{
public:

    Plane(Vec3 p, Vec3 n){
        position = p;
        direction = n;
    }

    bool intersect(const Ray ray, float& t) const override{
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
        return true;
    }

};

class Sphere : public virtual SceneObject{
public:
    Sphere(Vec3 c, float r){
        position = c;
        size = r;
    }

    bool intersect(const Ray ray, float& t) const override {
        Vec3 oc = ray.origin - position;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - pow(size, 2);
        float discriminant = b * b - 4 * a * c;

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
            return true;
        }
    }
};

class Scene {
public:
    vector<SceneObject*> objects;

    bool intersect(const Ray& ray, float& closestT, Vec3& intersection_position) const {
        closestT = std::numeric_limits<float>::max();
        bool hit = false;

        for (const auto& obj : objects) {
            float t;
            if (obj->intersect(ray, t) && t < closestT) {
                closestT = t;
                hit = true;
                intersection_position = ray.origin + ray.direction * closestT;
            }
        }

        return hit;
    }
};


class Render{

public:
    string render;
    int height;
    int width;
    unordered_map<int, string> pixel_values;

    long int frame_count;
    double render_time;
    bool skip_cout;


    Camera camera;
    Scene scene;
    Vec3 sun;

    Render(){
        this->render = "";
        this->width = 98;
        this->height = 50;
        this->pixel_values = ext_pixel_value;
        this->skip_cout = false;
        this->frame_count = 0;

        this->scene.objects.push_back( new Sphere({0,2,0}, 2));
        //this->scene.objects.push_back( new Sphere({2,2,2}, 2));
        this->scene.objects.push_back( new Plane({0,0,0},{0,1,0}));
        this->sun = {1,1,0};
        this->sun.normalize();

        this->camera.position = {0, 1.7, 0};
        this->camera.yaw = 1;
        this->camera.pitch = 0;
        this->camera.updateDirection();
        this->camera.fov = 100;
        system("clear");
    }

    void bufferDraw(int value){
        //int max_value = this->pixel_values.size();
        string pixel = this->pixel_values[clamp(value, 0, 4)];
        this->render += pixel;
    }

    void bufferNextLine(){
        this->render += "\n";
    }

    void bufferClear(){
        this->render = "";
    }

    void screenClear(){
        system("clear");
    }

    void moveCursor(int x, int y) {
        cout << "\033[" << y << ";" << x << "H";
    }

    void present(){

        moveCursor(1, 1);

        if (skip_cout){return;}
        cout << this->render;
        //cout.flush();
    }

    void main(){
        int color;
        int shadow;
        Vec3 intersection_point;
        Vec3 intersection_point_offset_direction;
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

                ray_collided = this->scene.intersect(ray, distance, intersection_point);
                if (ray_collided){
                    color += (int)round(intersection_point.z) % 2;
                    color += (int)round(intersection_point.x) % 2;
                    //color = round(9-ceil(intersection_point.distance({0,0,0})));
                    //color = clamp(color, 0, 4);
                    color += 2;

                    intersection_point_offset_direction = intersection_point + this->sun * 0.05f;

                    ray = {intersection_point_offset_direction, this->sun};
                    ray_collided = this->scene.intersect(ray, distance, intersection_point);
                    if (ray_collided){
                        color -= 1;
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

    void mainLoop(){
        while (true){
            this->camera.position.z = sin((double)this->frame_count / 300.0) * 4;
            this->camera.position.x = cos((double)this->frame_count / 300.0) * 4;
            this->scene.objects[0]->size = this->scene.objects[0]->position.y = sin((double)this->frame_count / 100.0) + 1;
            this->camera.direction = camera.position.pointTo({0,1.7,0});
            this->main();
            cout << "camera y rotation: " << this->camera.direction.y << "";
            cout << " / fov: " << this->camera.fov;
            cout << "  |  \033[1mtotal fps: " << 1000/(this->render_time);
            cout << " / total render time: " << this->render_time << "ms\033[0m";
        }
    }
};


int main()
{
    Render render;
    render.skip_cout = false;
    render.camera.rotate(0,0);
    render.mainLoop();
    float fps = render.render_time;
    //render.screenClear();
    cout << fps << "\nyipee";
    return 0;
}
