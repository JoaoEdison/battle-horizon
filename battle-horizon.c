/*
  Battle Horizon is a 3D space battle game in Raylib
  Copyright (C) 2023  João Edison Roso Manica
  
  Battle Horizon is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  Battle Horizon is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; If not, see <http://www.gnu.org/licenses/>
*/
#include <raylib.h>
#include <raymath.h>
#include "defs.h"
#include "linked_list.c"

#define MAX_MAP_NAME_LEN 60

struct model {
	int model;
	Vector3 position, angles;
	float scale;
};

struct models_with_collisions {
	Model drawing;
	char pathname[MAX_MAP_NAME_LEN];
	char texturepath[MAX_MAP_NAME_LEN];
	int has_texture;
	struct list collision_list;
};

#define TRANFORM_SPHERE(current_model, pos_model) \
		Vector3Transform(pos_model, \
				MatrixMultiply( \
					MatrixMultiply( \
						MatrixScale(current_model->scale, current_model->scale, current_model->scale), \
						MatrixRotateXYZ(Vector3Scale(current_model->angles, DEG2RAD)) \
					), \
					MatrixTranslate(current_model->position.x, \
						current_model->position.y, \
						current_model->position.z) \
					) \
				);

void draw_collisions_wires(current_model, collisions)
struct model *current_model;
struct list *collisions;
{
	struct node *next;
	struct model *ptrm;
	Vector3 temp;

	for (next = collisions->first; next; next = next->next) {
		ptrm = (struct model*)next->data;
		temp = TRANFORM_SPHERE(current_model, ptrm->position)
		DrawSphereWires(temp, ptrm->scale * current_model->scale, 5, 5, GREEN);
	}
}

/* From the library */

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

#define CAMERA_MOUSE_MOVE_SENSITIVITY 0.3f

void UpdateMyCamera(Camera *camera, float deltatime)
{
    static float delta_vel_up = 0.0f;
    static float delta_vel_side = 0.0f;
    static float velocity = SPACECRAFT_SPEED;
    Vector2 mousePositionDelta = GetMouseDelta();

    CameraYaw2(camera, -mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY * deltatime, true);
    CameraPitch2(camera, -mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY * deltatime, true, true, false);
    
    if (IsKeyPressed(KEY_SPACE))
        velocity = velocity == SPACECRAFT_SPEED? SPACECRAFT_SPEED_MAX : SPACECRAFT_SPEED;
    camera->target.z -= velocity * deltatime;
    camera->position.z -= velocity * deltatime;
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

void DrawModelRotate(Model model, Vector3 position, Vector3 rotationAngle, float scale, Color tint)
{
    Matrix matScale = MatrixScale(scale, scale, scale);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    rotationAngle.x *= DEG2RAD;
    rotationAngle.y *= DEG2RAD;
    rotationAngle.z *= DEG2RAD;
    Matrix matRotation = MatrixRotateXYZ(rotationAngle);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((((float)color.r/255.0f)*((float)tint.r/255.0f))*255.0f);
        colorTint.g = (unsigned char)((((float)color.g/255.0f)*((float)tint.g/255.0f))*255.0f);
        colorTint.b = (unsigned char)((((float)color.b/255.0f)*((float)tint.b/255.0f))*255.0f);
        colorTint.a = (unsigned char)((((float)color.a/255.0f)*((float)tint.a/255.0f))*255.0f);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}
