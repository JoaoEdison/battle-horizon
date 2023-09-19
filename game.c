#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "spacecraft.c"

/* TODO:
 * tiros;
 * naves inimigas;
 * movimentos;
 * barra de vida;
 * menu;
 * texturas;
 * sons.
 * */

/* armazenar os tiros em um vetor estático, calcular o máximo de tiros possíveis
 * de acordo com a frequência e distância máxima*/

struct models_with_collisions *models;
short model_count;
struct list rocks = { 0 };
struct list enemies = { 0 };
BoundingBox new_spaceship1, new_spaceship2;

main()
{
	void load_map(), UpdateMyCamera();
	bool collision = false, prev_collision = false;
	int screen_width, screen_height;
	Camera camera;
	Matrix pos_spaceship;
	struct node *next;
	struct model *draw_model;
	int life = 5;
	
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
	
	while (!WindowShouldClose()) {
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
					draw_collisions_wires(draw_model, models);
				}
				for (next = enemies.first; next; next = next->next) {
					draw_model = (struct model*)next->data;
					DrawModelRotate(models[draw_model->model].drawing,
							draw_model->position,
							draw_model->angles,
							draw_model->scale,
							GRAY);
					draw_collisions_wires(draw_model, models);
				}
				DrawSphere((Vector3){camera.position.x+250.0f, camera.position.y+150.0f, camera.position.z-700.0f}, 100.0f, YELLOW);
				DrawModel(mod_spaceship, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
				/*
				DrawBoundingBox(new_spaceship1, PURPLE);
				DrawBoundingBox(new_spaceship2, BLACK);
				if (collision) {
					DrawSphere(new_spaceship1.min, 0.1f, RED);
					DrawSphere(new_spaceship1.max, 0.1f, ORANGE);
				} else {
					DrawSphere(new_spaceship1.min, 0.1f, BLUE);
					DrawSphere(new_spaceship1.max, 0.1f, GREEN);
				}
				*/
			EndMode3D();
			DrawFPS(10, 10);
		EndDrawing();
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
