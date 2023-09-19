#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "spacecraft.c"
#include "defs.h"

/* TODO:
 * movimentos das naves inimigas;
 * danos para os inimigos;
 * barra de vida;
 * menu;
 * texturas;
 * sons.
 * */

#define BULLET_SPEED_PER_SECOND 200.0f
#define FIRING_RATE_PER_SECOND 3.0f 

struct models_with_collisions *models;
short model_count;
struct list rocks = { 0 };
struct list enemies = { 0 };
struct list following = { 0 };
struct list shots = { 0 };
BoundingBox new_spaceship1, new_spaceship2;

#define INITIAL_DIST 10000.0f
#define LIMINAL (100.0f + 70.0f * following.size)

// aplicar o losango

struct enemy_spacecraft {
	struct model shape;
	unsigned char id;
	int life;
	float dist;
};

main()
{
	void load_map(), UpdateMyCamera();
	bool collision = false, prev_collision = false;
	int screen_width, screen_height;
	Camera camera;
	Matrix pos_spaceship;
	struct node *next, *curr;
	struct model *draw_model;
	int life = 5;
	Vector3 new_bullet, *ptrbul, aux_bullet, aux_model;
	double prev_time, now;
	float inc;
	struct enemy_spacecraft new_enemy;

	prev_time = GetTime();
	now = .0;

	new_enemy.life = 5;

	camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
	camera.target = (Vector3){0.0f, 4.5f,-1.0f };
	camera.up = (Vector3){ .0f, 6.0f, .0f };
	camera.fovy = 90.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(0, 0, "Batalha Espacial");
	screen_width = GetScreenWidth();
	screen_height = GetScreenHeight();
	ToggleFullscreen();
	SetTargetFPS(60);
	SetMousePosition(screen_width / 2, screen_height / 2);
	DisableCursor();

	Model mod_spaceship = LoadModel("modelos/spacecraft.glb");
	// o mínimo é a frente, esquerda e baixo
	// há uma rotação de 90 graus, no sentido anti-horário e no eixo y
	BoundingBox box_spaceship1 = {
		{ .46f,-0.2f,-2.0f},
		{-.9f,	0.1f, 2.0f}
	};
	BoundingBox box_spaceship2 = {
		{ 0.8f,0.1f,-0.5f},
		{-0.7f,0.8f, 0.5f}
	};

	load_map("teste4.map");
	
	SetRandomSeed(3);

	while (!WindowShouldClose()) {
		/*atualizar a camera de acordo com os frames???*/
		UpdateMyCamera(&camera, CAMERA_THIRD_PERSON);
		pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
						MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
		 		MatrixTranslate(camera.target.x, camera.target.y - 1.8f,
					camera.target.z + 0.5f));
		mod_spaceship.transform = pos_spaceship;
		new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
		new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
		new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
		new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
		
		collision = check_collisions();
		if (collision && !prev_collision) {
			prev_collision = true;
			life--;
		}
		if (!collision)
			prev_collision = false;

		for (next = shots.first; next;) {
			ptrbul = (Vector3*)next->data;
			if (ptrbul->z > -2 * ARRIVAL_DIST) {
				ptrbul->z -= BULLET_SPEED_PER_SECOND * GetFrameTime();
				next = next->next;
			} else {
				curr = next;
				next = next->next;
				list_remove(curr, &shots);
			}
		}

		BeginDrawing();
			ClearBackground(RAYWHITE);
			BeginMode3D(camera);
				for (next = rocks.first; next; next = next->next) {
					draw_model = (struct model*)next->data;
					DrawModelRotate(models[draw_model->model].drawing,
							draw_model->position,
							draw_model->angles,
							draw_model->scale,
							GRAY);
					draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);
				}
				for (next = enemies.first; next;) {
					draw_model = (struct model*)next->data;
					if (draw_model->position.z + LIMINAL - camera.target.z >= .0f) {
						new_enemy.id = GetRandomValue(0, 2);
						new_enemy.shape = *draw_model;
						new_enemy.dist = camera.target.z - draw_model->position.z;
						list_insert(&new_enemy, &following, sizeof(struct enemy_spacecraft));
						curr = next;
						next = next->next;
						list_remove(curr, &enemies);
						continue;
					}
					inc = INITIAL_DIST * fabs(draw_model->position.z + LIMINAL - camera.target.z) /
						                                  (-draw_model->position.z);
					aux_model = draw_model->position;
					aux_model.x = aux_model.x > .0f? aux_model.x + inc : aux_model.x - inc;
					aux_model.y = aux_model.y > .0f? aux_model.y + inc : aux_model.y - inc;
					DrawModelRotate(models[draw_model->model].drawing,
							aux_model,
							draw_model->angles,
							draw_model->scale,
							GRAY);
					next = next->next;
				}
				for (next = following.first; next; next = next->next) {
					draw_model = &((struct enemy_spacecraft*)next->data)->shape;
					draw_model->position.z -= ((struct enemy_spacecraft*)next->data)->dist - fabs(camera.target.z - draw_model->position.z);
					DrawModelRotate(models[draw_model->model].drawing,
							draw_model->position,
							draw_model->angles,
							draw_model->scale,
							GRAY);
					draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);
				}
				DrawSphere((Vector3){camera.position.x+250.0f, camera.position.y+150.0f, camera.position.z-700.0f}, 100.0f, YELLOW);
				DrawModel(mod_spaceship, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
				for (next = shots.first; next; next = next->next) {
					aux_bullet = new_bullet = *(Vector3*)next->data;
					new_bullet.z += .2f;
					aux_bullet.z -= .4f;
					DrawCapsule(new_bullet, aux_bullet, .125f, 4, 4, ORANGE);
				}
			EndMode3D();
			DrawFPS(10, 10);
		EndDrawing();
		
		now = GetTime();
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
		    now - prev_time > 1.0f / FIRING_RATE_PER_SECOND) {
			new_bullet.x = camera.target.x-1.58f;
			new_bullet.y = camera.target.y-1.926f;
			new_bullet.z = camera.target.z - 0.62 - .4f / 2;
			list_insert(&new_bullet, &shots, sizeof(Vector3));
			new_bullet.x += 3.22f;
			new_bullet.y -= 0.05f;
			new_bullet.z -= 0.03f;
			list_insert(&new_bullet, &shots, sizeof(Vector3));
			prev_time = GetTime();
		}
	}
	CloseWindow();
}

void load_map(name)
char name[];
{
	struct model new;
	short i, j, enemy_idx;
	FILE *fp;
	
	enemy_idx = -1;
	fp = fopen(name, "r");
	fscanf(fp, "%hd\n", &model_count);
	models = malloc(sizeof(struct models_with_collisions) * model_count);
	for (i=0; i < model_count; i++) {
		fscanf(fp, "%s %hd\n", models[i].pathname, &j);
		models[i].drawing = LoadModel(models[i].pathname);
		if (strstr(models[i].pathname, "enemy"))
			enemy_idx = i;
		models[i].collision_list.first = NULL;
		models[i].collision_list.size = 0;
		while (j--) {
			fscanf(fp, "%f %f %f %f\n",
					&new.position.x,
					&new.position.y,
					&new.position.z,
					&new.scale);
			list_insert(&new, &models[i].collision_list, sizeof(struct model));
		}
	}
	fscanf(fp, "%hd\n", &i);
	while (i--) {
		fscanf(fp, "%hhu\n", &new.model);
		fscanf(fp, "%f %f %f %f\n",
				&new.position.x,
				&new.position.y,
				&new.position.z,
				&new.scale);
		fscanf(fp, "%f %f %f\n", &new.angles.x,
				&new.angles.y, &new.angles.z);
		list_insert(&new, enemy_idx == new.model? &enemies : &rocks, sizeof(struct model));
	}
	fclose(fp);
}

check_collisions()
{
	struct node *next, *next_box;
	struct list *collisions_list;
	struct model *ptrm, *current;
	Vector3 temp;
	float new_scale;

	for (next = rocks.first; next; next = next->next) {
		current = (struct model*)next->data;
		collisions_list = &models[current->model].collision_list;
		for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
			ptrm = (struct model*)next_box->data;
			new_scale = ptrm->scale * current->scale;
			temp = TRANFORM_SPHERE(current);
			if (CheckCollisionBoxSphere(new_spaceship1, temp, new_scale))
				return 1;
			if (CheckCollisionBoxSphere(new_spaceship2, temp, new_scale))
				return 1;
		}
	}
	return 0;
}
