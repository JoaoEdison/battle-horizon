#include <raylib.h>
#include <raymath.h>
#include "linked_list.c"

#define MAX_MAP_NAME_LEN 40

struct model {
	unsigned char model;
	Vector3 position, angles;
	float scale;
};

struct models_with_collisions {
	char pathname[MAX_MAP_NAME_LEN];
	Model drawing;
	struct list collision_list;
};

#define TRANFORM_SPHERE(current_model) \
		Vector3Transform(ptrm->position, \
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

void draw_collisions_wires(current_model, models)
struct model *current_model;
struct models_with_collisions models[];
{
	struct list *collisions;
	struct node *next;
	struct model *ptrm;
	float new_scale;
	Vector3 temp;

	collisions = &models[current_model->model].collision_list;
	for (next = collisions->first; next; next = next->next) {
		ptrm = (struct model*)next->data;
		temp = TRANFORM_SPHERE(current_model)
		new_scale = ptrm->scale * current_model->scale;
		DrawSphereWires(temp, new_scale, 5, 5, GREEN);
	}
}
