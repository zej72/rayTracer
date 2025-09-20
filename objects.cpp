#include "rayTracer.h"

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
