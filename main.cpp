#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <optional>
#include <thread>
#include <chrono>

using namespace std;

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

struct Plane {
    Vec3 point;   // A point on the plane
    Vec3 normal;  // The normal vector of the plane
};

struct Sphere {
    Vec3 center;
    float radius;
};

bool rayIntersectsPlane(const Ray ray, const Plane plane, Vec3& intersectionPoint, float& t) {
    float denominator = ray.direction.dot(plane.normal);

    // If the denominator is close to zero, the ray is parallel to the plane
    if (fabs(denominator) < 1e-6) {
        return false; // No intersection
    }

    Vec3 difference = plane.point - ray.origin;
    t = difference.dot(plane.normal) / denominator;

    // If t is negative, the intersection point is behind the ray's origin
    if (t < 0) {
        return false; // No intersection
    }

    // Calculate the intersection point
    intersectionPoint = ray.origin + ray.direction * t;
    return true; // Intersection occurred
}

bool rayIntersectsSphere(const Ray ray, const Sphere sphere, Vec3& intersectionPoint, float& t) {
    Vec3 oc = ray.origin - sphere.center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false; // No intersection
    } else {
        // Calculate the nearest intersection point
        float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + std::sqrt(discriminant)) / (2.0f * a);

        // We want the closest positive t
        float t = (t1 > 0) ? t1 : t2;

        if (t < 0) {
            return false; // Intersection is behind the ray's origin
        }

        // Calculate the intersection point
        intersectionPoint = ray.origin + ray.direction * t;
        return true; // Return the intersection point
    }
}


class Render{

public:
    string render;
    int height;
    int width;
    unordered_map<int, string> pixel_values;

    long int frame_count;
    double render_time;
    double rtx_render_time;
    bool skip_cout;


    Camera camera;
    Plane ground;
    Sphere sphere;
    Vec3 sun;

    Render(){
        this->render = "";
        this->render.reserve(4095);
        this->width = 88;
        this->height = 40;
        this->pixel_values = {
            {0, " "},
            {1, "-"},
            {2, ":"},
            {3, "/"},
            {4, "["},
            {5, "U"},
            {6, "8"},
            {7, "%"},
            {8, "M"},
            {9, "@"}
        };

        this->skip_cout = false;

        this->frame_count = 0;
        this->ground = {{0, 0, 0}, {0, 1, 0}};
        this->sphere = {{3,2,0}, 2};
        this->sun = {1,3,0};

        this->camera.position = {0, 1.7, 0};
        this->camera.yaw = 1;
        this->camera.pitch = 0;
        this->camera.updateDirection();
        this->camera.fov = 100;
        system("clear");
    }

    void bufferDraw(int value){
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
        Vec3 intersection_position;
        Ray ray;
        Ray sub_ray;
        bool ray_collided;
        float distance;

        this->bufferClear();

        chrono::steady_clock::time_point begin_rtx = chrono::steady_clock::now();

        for(int y = 0 ; y < this->height; ++y) {
            for(int x = 0; x < this->width*2; ++x) {
                color = 0;

                ray = this->camera.getRay((float)x/2.0f, y, this->width, this->height);

                ray_collided = rayIntersectsPlane(ray, ground, intersection_position, distance);
                if (ray_collided){
                    color = round(10-ceil(distance));
                    color += 1;
                    color = clamp(color, 1, 4);
                    sub_ray = {intersection_position,this->sun};
                }

                ray_collided = rayIntersectsSphere(ray, this->sphere, intersection_position, distance);
                if(ray_collided){
                    color = 4;
                    sub_ray = {intersection_position, this->sun};
                }

                bool add_shadows = color != 0 && rayIntersectsSphere(sub_ray, this->sphere, intersection_position, distance);
                if (add_shadows){
                    //shadow = 1 - round(distance);
                    color = 0;
                }

                this->bufferDraw(color);
            }
            this->bufferNextLine();
        }

        chrono::steady_clock::time_point end_rtx = chrono::steady_clock::now();
        chrono::steady_clock::time_point begin = chrono::steady_clock::now();
        this->present();
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        this->render_time = chrono::duration_cast<chrono::nanoseconds> (end - begin).count() / 1000000.0;
        this->rtx_render_time = chrono::duration_cast<chrono::nanoseconds> (end_rtx - begin_rtx).count() / 1000000.0;
        this->frame_count++;
    }

    void mainLoop(){
        while (true){
            //this->camera.rotate(0.1,0);
            this->sphere.center.z = sin((float)this->frame_count/100.0f);
            this->main();
            cout << "camera y rotation: " << this->camera.direction.y << "";
            cout << " / fov: " << this->camera.fov;
            cout << "  |  \033[1mtotal fps: " << 1000/((this->render_time + this->rtx_render_time));
            cout << " / total render time: " << (this->render_time + this->rtx_render_time) << "ms\033[0m";
            cout << "  |  cout fps: " << 1000/(this->render_time);
            cout << " / cout time: " << this->render_time << "ms";
            cout << "  |  rtx fps: " << 1000/(this->rtx_render_time) << "fps";
            cout << " / rtx render time: " << this->rtx_render_time << "ms";
        }
    }
};


int main()
{
    Render render;
    //render.skip_cout = true;
    render.camera.rotate(0,0);
    render.mainLoop();
    float fps = (render.rtx_render_time + render.render_time);
    //render.screenClear();
    cout << fps << "\nyipee";
    return 0;
}
