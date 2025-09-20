#include "rayTracer.h"

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
