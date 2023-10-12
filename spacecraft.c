/*
  Spacecraft is a 3D space battle game in Raylib
  Copyright (C) 2023  João Edison Roso Manica
  
  Spacecraft is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  Spacecraft is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; If not, see <http://www.gnu.org/licenses/>
*/
#include <raylib.h>
#include <raymath.h>
#include "linked_list.c"

#define MAX_MAP_NAME_LEN 60

struct model {
	unsigned char model;
	Vector3 position, angles;
	float scale;
};

struct models_with_collisions {
	Model drawing;
	char pathname[MAX_MAP_NAME_LEN];
	char texturepath[MAX_MAP_NAME_LEN];
	unsigned char has_texture;
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
