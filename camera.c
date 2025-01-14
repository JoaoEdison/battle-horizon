#include <raylib.h>
#include <raymath.h>
#include "defs.h"

/* --- From the library --- */

// Returns the cameras forward vector (normalized)
Vector3 GetCameraForward2(Camera *camera)
{
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
Vector3 GetCameraUp2(Camera *camera)
{
    return Vector3Normalize(camera->up);
}

// Returns the cameras right vector (normalized)
Vector3 GetCameraRight2(Camera *camera)
{
    Vector3 forward = GetCameraForward2(camera);
    Vector3 up = GetCameraUp2(camera);

    return Vector3CrossProduct(forward, up);
}

void CameraYaw2(Camera *camera, float angle, bool rotateAroundTarget)
{
    // Rotation axis
    Vector3 up = GetCameraUp2(camera);

    // View vector
    Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

    // Rotate view vector around up axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, up, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, targetPosition);
    }
}

// Rotates the camera around its right vector, pitch is "looking up and down"
//  - lockView prevents camera overrotation (aka "somersaults")
//  - rotateAroundTarget defines if rotation is around target or around its position
//  - rotateUp rotates the up direction as well (typically only usefull in CAMERA_FREE)
// NOTE: angle must be provided in radians
void CameraPitch2(Camera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp)
{
    // Up direction
    Vector3 up = GetCameraUp2(camera);

    // View vector
    Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

    if (lockView)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.

        // Clamp view up
        float maxAngleUp = Vector3Angle(up, targetPosition);
        maxAngleUp -= 0.001f; // avoid numerical errors
        if (angle > maxAngleUp) angle = maxAngleUp;

        // Clamp view down
        float maxAngleDown = Vector3Angle(Vector3Negate(up), targetPosition);
        maxAngleDown *= -1.0f; // downwards angle is negative
        maxAngleDown += 0.001f; // avoid numerical errors
        if (angle < maxAngleDown) angle = maxAngleDown;
    }

    // Rotation axis
    Vector3 right = GetCameraRight2(camera);

    // Rotate view vector around right axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, right, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, targetPosition);
    }

    if (rotateUp)
    {
        // Rotate up direction around right axis
        camera->up = Vector3RotateByAxisAngle(camera->up, right, angle);
    }
}

// Moves the camera in its forward direction
void CameraMoveForward2(Camera *camera, float distance)
{
    Vector3 forward = GetCameraForward2(camera);

    // Scale by distance
    forward = Vector3Scale(forward, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, forward);
    camera->target = Vector3Add(camera->target, forward);
}

// Moves the camera target in its current right direction
void CameraMoveRight2(Camera *camera, float distance)
{
    Vector3 right = GetCameraRight2(camera);

    // Scale by distance
    right = Vector3Scale(right, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, right);
    camera->target = Vector3Add(camera->target, right);
}

/* ------------------------ */

#define CAMERA_MOUSE_MOVE_SENSITIVITY 0.3f
#define ACCELERATION 2.0f
#define ZERO_VEL \
	velocitySpaceship = SPACECRAFT_SPEED; \
	turbo = false; \
	acceleration = 0.0f;

float velocitySpaceship = SPACECRAFT_SPEED;
float acceleration = 0.0f;
bool turbo = false;

void UpdateMyCamera(Camera *camera, float deltatime)
{
    static float delta_vel_up = 0.0f;
    static float delta_vel_side = 0.0f;
    
    Vector2 mousePositionDelta = GetMouseDelta();

    CameraYaw2(camera, -mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY * deltatime, true);
    CameraPitch2(camera, -mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY * deltatime, true, true, false);
    
    if (IsKeyPressed(KEY_SPACE)) {
	turbo = !turbo;
        velocitySpaceship = turbo? SPACECRAFT_SPEED_MAX : SPACECRAFT_SPEED;
    }
    camera->target.z -= (velocitySpaceship+acceleration) * deltatime;
    camera->position.z -= (velocitySpaceship+acceleration) * deltatime;
    acceleration += ACCELERATION * deltatime;
    if (IsKeyDown(KEY_W)) {
        if (delta_vel_up < 0.0f)
            delta_vel_up = 0.0f;
        else if (delta_vel_up > LIMIT_VELOCITY)
            delta_vel_up = LIMIT_VELOCITY;
        if (camera->target.y + delta_vel_up * deltatime <  MAX_DIST + 1.8f)
            delta_vel_up += VELOCITY_FORWARD;
        else
            delta_vel_up =  MAX_DIST + 1.8f - camera->target.y;
        camera->target.y += delta_vel_up * deltatime;
        camera->position.y += delta_vel_up * deltatime;
    }
    if (IsKeyDown(KEY_S)) {
        if (delta_vel_up > 0.0f)
            delta_vel_up = 0.0f;
        else if (delta_vel_up < -LIMIT_VELOCITY)
            delta_vel_up = -LIMIT_VELOCITY;
        if (camera->target.y + delta_vel_up * deltatime > -MAX_DIST + 1.8f)
            delta_vel_up -= VELOCITY_FORWARD;
        else
            delta_vel_up = -MAX_DIST + 1.8f - camera->target.y;
        camera->target.y += delta_vel_up * deltatime;
        camera->position.y += delta_vel_up * deltatime;
    }
    if (IsKeyDown(KEY_D)) {
        if (delta_vel_side < 0.0f)
            delta_vel_side = 0.0f;
        else if (delta_vel_side > LIMIT_VELOCITY)
            delta_vel_side = LIMIT_VELOCITY;
        if (camera->target.x + delta_vel_side * deltatime <  MAX_DIST)
            delta_vel_side += VELOCITY_SIDE;
        else
            delta_vel_side =  MAX_DIST - camera->target.x;
        camera->target.x += delta_vel_side * deltatime;
        camera->position.x += delta_vel_side * deltatime; 
    }
    if (IsKeyDown(KEY_A)) {
        if (delta_vel_side > 0.0f)
            delta_vel_side = 0.0f;
        else if (delta_vel_side < -LIMIT_VELOCITY)
            delta_vel_side = -LIMIT_VELOCITY;
        if (camera->target.x + delta_vel_side * deltatime > -MAX_DIST)
            delta_vel_side -= VELOCITY_SIDE;
        else
            delta_vel_side = -MAX_DIST - camera->target.x;
        camera->target.x += delta_vel_side * deltatime;
        camera->position.x += delta_vel_side * deltatime; 
    }
    if (IsKeyReleased(KEY_S) || IsKeyReleased(KEY_W))
        delta_vel_up = 0.0f;
    if (IsKeyReleased(KEY_A) || IsKeyReleased(KEY_D))
        delta_vel_side = 0.0f;
}

#define CAMERA_MOUSE_MOVE_SENSITIVITY_EDITOR 0.3f
float velocityCameraEditor = CAMERA_SPEED_EDITOR;

void UpdateCameraEditor(Camera *camera, float deltatime)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    CameraYaw2(camera, -mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY_EDITOR * deltatime, false);
    CameraPitch2(camera, -mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY_EDITOR * deltatime, true, false, false);
    if (IsKeyPressed(KEY_SPACE)) {
		turbo = !turbo;
        velocityCameraEditor = turbo? CAMERA_SPEED_MAX_EDITOR : CAMERA_SPEED_EDITOR;
    }
	if (IsKeyDown(KEY_W)) CameraMoveForward2(camera, velocityCameraEditor * deltatime);
	if (IsKeyDown(KEY_A)) CameraMoveRight2(camera, -velocityCameraEditor * deltatime);
	if (IsKeyDown(KEY_S)) CameraMoveForward2(camera, -velocityCameraEditor * deltatime);
	if (IsKeyDown(KEY_D)) CameraMoveRight2(camera, velocityCameraEditor * deltatime);
}
