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

#define INPUT_QTT 7

struct enemy_spacecraft {
	struct model shape;
	Color color;
	float dist;
	int life;
	float last_state[INPUT_QTT];
	struct {
		unsigned collision : 1;
		unsigned shooted : 1;
		unsigned error : 1;
		unsigned dont_shoot : 1;
	} penalties;
	float last_shoot;
	unsigned char ammunition, has_penalty;
	struct list bullets;
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

void draw_collisions_wires(current_model, collisions)
struct model *current_model;
struct list *collisions;
{
	struct node *next;
	struct model *ptrm;
	Vector3 temp;

	for (next = collisions->first; next; next = next->next) {
		ptrm = (struct model*)next->data;
		temp = TRANFORM_SPHERE(current_model)
		DrawSphereWires(temp, ptrm->scale * current_model->scale, 5, 5, GREEN);
	}
}
