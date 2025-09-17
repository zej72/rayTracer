#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <optional>
#include <thread>
#include <chrono>
#include <vector>
#include <future>

using namespace std;

struct Vec3 {
public:
    float x, y, z;

    Vec3 operator+(const Vec3& other) const;

    Vec3 operator-(const Vec3& other) const;

    Vec3 operator*(float scalar) const;

    Vec3 normalize() const;

    float dot(const Vec3& other) const;

    Vec3 cross(const Vec3& other) const;

    float distance(const Vec3& other) const;

    Vec3 pointTo(const Vec3& other) const;

    float angle(const Vec3& other) const;
};


struct Ray {
public:
    Vec3 origin;
    Vec3 direction;
};

class Camera {
public:
    Vec3 position;
    Vec3 direction;
    Vec3 up;
    float fov;
    float yaw;
    float pitch;

    void rotate(float deltaYaw, float deltaPitch);

    void updateDirection();

    Ray getRay(float x, float y, int width, int height);
};

class SceneObject {
public:
    string name;
    string ANSI;
    bool cast_shadow;
    Vec3 position;
    Vec3 direction;
    float size;
    float size2;
    virtual bool intersect(const Ray ray, float& t, Vec3& n) const = 0;
    virtual ~SceneObject(){}
};

class Plane : public virtual SceneObject{
public:
    Plane(Vec3 p, Vec3 n, float s);

    bool intersect(const Ray ray, float& t, Vec3& n) const;
};

class Sphere : public virtual SceneObject{
public:
    Sphere(Vec3 c, float r);

    bool intersect(const Ray ray, float& t, Vec3& n) const;
};

class Ring : public virtual SceneObject{
public:
    Ring(Vec3 c, Vec3 n, float in_s, float out_s);

    bool intersect(const Ray ray, float& t, Vec3& n) const;
};

struct Render_data{
public:
	Ray ray;
	Vec3 point;
	Vec3 normal;
	float distance;
	string ANSI;
	bool shadow_rendering;
	int color;
};

class Scene {
public:
    vector<SceneObject*> objects;

    // nvm it was so shit i couldnt leave it like this
    bool intersect(Render_data& data) const;
};

class RayTracer{

public:
    string frame;
    int height;
    int width;
    unordered_map<int, string> pixel_values;
    int thread_count;

    long int frame_count;
    double render_time;
    bool skip_cout;

    Camera camera;
    Scene scene;
    Vec3 sun;

    RayTracer();
    void bufferDraw(int value);
    void bufferNextLine();
    void bufferClear();
    void screenClear();
    void moveCursor(int x, int y);
    void present();
    void render();
    string main(int start, int end);
};

#endif // RAYTRACER_H
