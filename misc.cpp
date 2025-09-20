#include "rayTracer.h"

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
