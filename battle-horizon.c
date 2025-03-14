/*
  Battle Horizon is a 3D space battle game in Raylib
  Copyright (C) 2023-2025  João E. R. Manica
  
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

typedef struct {
	int model_idx;
	Vector3 position, angles;
	float scale;
} model;

typedef struct {
	Model drawing;
	char pathname[MAX_MAP_NAME_LEN];
	char texturepath[MAX_MAP_NAME_LEN];
	int has_texture;
	list_linkedlist collision_list;
} models_with_collisions;

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
model *current_model;
list_linkedlist *collisions;
{
	node_linkedlist *next;
	model *ptrm;
	Vector3 temp;

	for (next = collisions->first; next; next = next->next) {
		ptrm = (model*)next->data;
		temp = TRANFORM_SPHERE(current_model, ptrm->position)
		DrawSphereWires(temp, ptrm->scale * current_model->scale, 5, 5, GREEN);
	}
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
