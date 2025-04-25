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
    virtual bool intersect(const Ray ray, float& t) const = 0;
    virtual ~SceneObject(){}
};

struct Plane : public SceneObject{
public:
    Vec3 point;   // A point on the plane
    Vec3 normal;  // The normal vector of the plane

    Plane(Vec3 p, Vec3 n) : point(p), normal(n) {}

    bool intersect(const Ray ray, float& t) const override{
        float denominator = ray.direction.dot(normal);

        // If the denominator is close to zero, the ray is parallel to the plane
        if (fabs(denominator) < 1e-6) {
            return false; // No intersection
        }

        Vec3 difference = point - ray.origin;
        t = difference.dot(normal) / denominator;

        // If t is negative, the intersection point is behind the ray's origin
        if (t < 0.001f) {
            return false;
        }
        return true;
    }

};

struct Sphere : public SceneObject{
public:
    Vec3 center;
    float radius;

    Sphere(Vec3 c, float r) : center(c), radius(r){}

    bool intersect(const Ray ray, float& t) const override {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
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
        this->render.reserve(4095);
        this->width = 96;
        this->height = 46;
        this->pixel_values = ext_pixel_value;
        this->skip_cout = false;
        this->frame_count = 0;

        this->scene.objects.push_back( new Sphere({3,2,0}, 2));
        this->scene.objects.push_back( new Plane({0,0,0},{0,1,0}));
        this->sun = {0.3,1,0};

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
        float ray_position;
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
                    color = (int)round(intersection_point.z) % 2;
                    //color = round(9-ceil(intersection_point.distance({0,0,0})));
                    //color = clamp(color, 0, 4);
                    color += 2;

                    intersection_point_offset_direction = intersection_point + this->sun * 0.05f;

                    ray = {intersection_point_offset_direction, this->sun};
                    ray_collided = this->scene.intersect(ray, distance, intersection_point);
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

    void mainLoop(){
        while (true){
            this->camera.position.z = sin((double)this->frame_count / 100.0);
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
    //render.skip_cout = true;
    render.camera.rotate(0,0);
    render.mainLoop();
    float fps = render.render_time;
    //render.screenClear();
    cout << fps << "\nyipee";
    return 0;
}
