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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include "defs.h"
#include "battle-horizon.c"

#define MAX_MODELS 10

struct models_with_collisions models[MAX_MODELS];
short model_count;
int enemy_model;

#define NEW_MODEL new_model.position = camera.target; \
		  new_model.angles = (Vector3){ .0f, .0f, .0f }; \
		  new_model.scale = 10.0f;

void load_models()
{
	struct model new_box = { 0 };
	int i;
	
	i = 0;
	strcpy(models[i].pathname, "models/asteroid1.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	strcpy(models[i].texturepath, "models/black-white-details-moon-texture.png");
	models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
	models[i].has_texture = 1;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 1.0f;
	new_box.position = (Vector3){ 0 };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	
	i++;
	strcpy(models[i].pathname, "models/asteroid2.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	strcpy(models[i].texturepath, "models/black-white-details-moon-texture.png");
	models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
	models[i].has_texture = 1;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 1.0f;
	new_box.position = (Vector3){ 0 };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	
	i++;
	strcpy(models[i].pathname, "models/asteroid3.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	strcpy(models[i].texturepath, "models/black-stone-texture.png");
	models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
	models[i].has_texture = 1;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = .8f;
	new_box.position = (Vector3){ -.2f, .4f, .2f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = .9f;
	new_box.position = (Vector3){ .2f, -.2f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));

	
	i++;
	strcpy(models[i].pathname, "models/asteroid4.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	strcpy(models[i].texturepath, "models/black-stone-texture.png");
	models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
	models[i].has_texture = 1;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 0.7f;
	new_box.position = (Vector3){ 0.0f, 0.0f, 0.2f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 0.6f;
	new_box.position = (Vector3){ 0.0f,-0.5f, 0.0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	
	i++;
	strcpy(models[i].pathname, "models/asteroid5.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	strcpy(models[i].texturepath, "models/black-stone-texture.png");
	models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
	models[i].has_texture = 1;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 0.7f;
	new_box.position = (Vector3){ 0 };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 0.6f;
	new_box.position = (Vector3){ 0.0f,  0.5f, 0.2f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	
	i++;
	strcpy(models[i].pathname, "models/asteroid6.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	models[i].has_texture = 0;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 1.5f;
	new_box.position = (Vector3){ 0 };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 1.5f;
	new_box.position = (Vector3){ 0.0f, 1.0f, 0.0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 1.5f;
	new_box.position = (Vector3){ 0.0f, -1.0f, 0.0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	
	i++;
	enemy_model = i;
	strcpy(models[i].pathname, "models/enemy.glb");
	models[i].drawing = LoadModel(models[i].pathname);
	models[i].has_texture = 0;
	models[i].collision_list.first = NULL;
	models[i].collision_list.size = 0;
	new_box.scale = 1.5f;
	new_box.position = (Vector3){ 0 };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 1.1f;
	new_box.position = (Vector3){ 2.3f, .0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = 1.1f;
	new_box.position = (Vector3){-2.3f, .0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = .8f;
	new_box.position = (Vector3){ 2.1f, 2.0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = .8f;
	new_box.position = (Vector3){-2.1f, 2.0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = .8f;
	new_box.position = (Vector3){ 2.2f,-2.0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));
	new_box.scale = .8f;
	new_box.position = (Vector3){-2.2f,-2.0f, .0f };
	list_insert(&new_box, &models[i].collision_list, sizeof(struct model));

	model_count = 7;
}

struct list drawing = { 0 };
Camera camera = { 0 };

struct group {
	Vector3 position;
	float radius;
	struct list group_selection;
} new_group = { 0 };

struct list groups = { 0 };
struct list selection = { 0 };
struct node *selected;
struct list ln = { 0 };
Vector3 lastpos;

bool save_flag = false, open_flag = false, move_flag = false;
char name[MAX_MAP_NAME_LEN];

struct model new_model;

main()
{
	void map_edit(), add_objects(), draw_scene(), get_in_volume();
	void trans_single(), trans_selection(), trans_group();
	struct node *get_selected();
	struct node *next, *curr;
	
	new_group.radius = 1.0f;

	*name = '\0';
	selected = NULL;

	camera.position = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, -15.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 90.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(0, 0, "Editor - Batalha Espacial");
	SetTargetFPS(60);
	DisableCursor();
 	
	load_models();	

	while (!WindowShouldClose()) {
		if (save_flag || open_flag)
			map_edit();
		else {
			UpdateCamera(&camera, CAMERA_FREE);
			add_objects();
			if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
				if (move_flag)
					move_flag = false;
				else {
					move_flag = true;
					lastpos = camera.target;
				}
			}
			if (selected)
				trans_single();
			if (selection.size)
				trans_selection();
			if (groups.size)
				trans_group();
			if (IsKeyDown(KEY_LEFT_CONTROL)) {
				if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
					if (selected) {
						list_insert(selected->data, &selection, sizeof(struct model));
						list_remove(selected, &drawing);
						selected = NULL;
					}
					if ((next = get_selected(&drawing))) {
						list_insert(next->data, &selection, sizeof(struct model));
						list_remove(next, &drawing);
					}
				} else if (IsKeyPressed(KEY_A))
					for (next=drawing.first; next;) {
						if (((struct model*)next->data)->model != enemy_model) {
							list_insert(next->data, &selection, sizeof(struct model));
							curr = next;
							next = next->next;				
							list_remove(curr, &drawing);
							continue;
						}
						next = next->next;
					}
				else if (IsKeyPressed(KEY_S))
					save_flag = true;
				else if (IsKeyPressed(KEY_O))
					open_flag = true;
			} else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
				selected = get_selected(&drawing);
				if (!selected)
					while (selection.first) {
						list_insert(selection.last->data, &drawing, sizeof(struct model));
						list_remove(selection.last, &selection);
					}
			}
		}
		draw_scene();
	}
}

void add_objects()
{
	char number_pressed;
	
	number_pressed = -1; 
	if (IsKeyPressed(KEY_ONE))
		number_pressed = 0;
	if (IsKeyPressed(KEY_TWO))
		number_pressed = 1;
	if (IsKeyPressed(KEY_THREE))
		number_pressed = 2;
	if (IsKeyPressed(KEY_FOUR))
		number_pressed = 3;
	if (IsKeyPressed(KEY_FIVE))
		number_pressed = 4;
	if (IsKeyPressed(KEY_SIX))
		number_pressed = 5;
	if (IsKeyPressed(KEY_SEVEN))
		number_pressed = 6;
	if (number_pressed >= 0) {
		NEW_MODEL
		new_model.model = number_pressed;
		list_insert(&new_model, &drawing, sizeof(struct model));
	}
	if (IsKeyPressed(KEY_G)) {
		new_group.position = camera.target;
		list_insert(&new_group, &groups, sizeof(struct group));
	}
}

void map_edit()
{
	void save_map(), open_map();
	static char real_name[MAX_MAP_NAME_LEN+9];
	static letter_count = 0;
	int key;

	while ((key = GetCharPressed()))
		if (letter_count < MAX_MAP_NAME_LEN-1 && key != KEY_ENTER && key != KEY_BACKSPACE) {
			name[letter_count++] = key;
			name[letter_count] = '\0';
		}
	if (IsKeyDown(KEY_BACKSPACE)) {
		if (letter_count) {
			letter_count--;
			name[letter_count] = '\0';
		}
	} else if (IsKeyPressed(KEY_ENTER)) {
		if (letter_count) {
			strcpy(real_name, "data/");
			strcat(real_name, name);
			strcat(real_name, ".map");
			if (save_flag) {
				save_map(real_name);
				*name = letter_count = save_flag = 0;

			} else if (FileExists(real_name)) {
				open_map(real_name);
				*name = letter_count = open_flag = 0;
			} else {
				strcpy(name, "Arquivo Inexistente!");
				letter_count = 20; 
			}
		} else
			open_flag = save_flag = 0;
	}
}

struct node *get_selected(l)
struct list *l;
{
	struct list *collisions;
	struct node *next, *next_collision;
	struct model *target;
	struct model *ptrm;
	float new_scale;
	Vector3 temp;

	for (next = l->first; next; next = next->next) {
		target = (struct model*)next->data;
		collisions = &models[target->model].collision_list;
		for (next_collision = collisions->first; next_collision; next_collision = next_collision->next) {
			ptrm = (struct model*)next_collision->data;
			temp = TRANFORM_SPHERE(target, ptrm->position)
			new_scale = ptrm->scale * target->scale;
			if (CheckCollisionSpheres(temp, new_scale, camera.target, .3f))
				return next;
		}
	}
	return NULL;
}

/*salvar em um arquivo temporário?*/
void save_map(name)
char *name;
{
	struct node *next1, *next2;
	struct model *current;
	FILE *fp;
	short i, count = 0, map[MAX_MODELS];
	struct list present = { 0 };
	
	for (i = 0; i < MAX_MODELS; i++)
		map[i] = -1;
	for (next1 = drawing.first; next1; next1 = next1->next) {
		i = ((struct model*)next1->data)->model;
		if (map[i] < 0) {
			map[i] = count++;
			list_insert(&i, &present, sizeof(short));
		}
	}
	fp = fopen(name, "wb");
	fprintf(fp, "%hd\n", count);
	for (next1 = present.first; next1; next1 = next1->next) {
		i = *(short*)next1->data;
		fprintf(fp, "%s %hd\n", models[i].pathname, models[i].collision_list.size);
		fprintf(fp, "%hhd %s\n", models[i].has_texture, models[i].has_texture? models[i].texturepath : "0");
		for (next2 = models[i].collision_list.first; next2; next2 = next2->next) {
			current = (struct model*)next2->data;
			fprintf(fp, "%.4f %.4f %.4f %.4f\n",
					current->position.x,
					current->position.y,
					current->position.z,
					current->scale);
		}
	}
	fprintf(fp, "%hd\n", drawing.size);
	for (next1 = drawing.first; next1; next1 = next1->next) {
		current = (struct model*)next1->data;
		fprintf(fp, "%hhu\n", map[current->model]);
		fprintf(fp, "%.4f %.4f %.4f %.4f\n",
				current->position.x,
				current->position.y,
				current->position.z,
				current->scale);
		fprintf(fp, "%.4f %.4f %.4f\n", current->angles.x,
				current->angles.y, current->angles.z);
	}
	fclose(fp);
	while (present.first)
		list_remove(present.first, &present);
}

void open_map(name)
char name[];
{
	struct model new;
	char model_name[MAX_MAP_NAME_LEN], texture_name[MAX_MAP_NAME_LEN];
	FILE *fp;
	short map[MAX_MODELS];
	short i, j, k, total_models;
	unsigned char has_texture;
	
	for (i = 0; i < MAX_MODELS; i++)
		map[i] = -1;	
	fp = fopen(name, "r");
	fscanf(fp, "%hd\n", &total_models);
	for (i=0; i < total_models; i++) {
		fscanf(fp, "%s %hd\n", model_name, &k);
		for (j=0; j < model_count; j++)
			if (!strcmp(models[j].pathname, model_name)) {
				if (strstr(model_name, "enemy"))
					enemy_model = j;
				map[i] = j;
				break;
			}
		fscanf(fp, "%hhd %s\n", &has_texture, texture_name);
		while (k--)
			while (fgetc(fp) != '\n');
	}
	fscanf(fp, "%hd\n", &j);
	while (j--) {
		fscanf(fp, "%hhu\n", &new.model);
		new.model = map[new.model];
		fscanf(fp, "%f %f %f %f\n",
				&new.position.x,
				&new.position.y,
				&new.position.z,
				&new.scale);
		fscanf(fp, "%f %f %f\n", &new.angles.x,
				&new.angles.y, &new.angles.z);
		list_insert(&new, &drawing, sizeof(struct model));
	}
	fclose(fp);
}

#define DRAW_MODEL(PTR, COLOR) \
		draw_model = (struct model*)PTR->data; \
		DrawModelRotate(models[draw_model->model].drawing, draw_model->position, draw_model->angles, draw_model->scale, COLOR); \
		draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);

void draw_scene()
{
	struct model *draw_model;
	struct node *next, *current;

	BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode3D(camera);
			for (next = drawing.first; next; next = next->next) {
				if (next == selected) {
					DRAW_MODEL(next, PURPLE)
				} else {
					DRAW_MODEL(next, GRAY)	
				}
			}
			for (next = selection.first; next; next = next->next) {
				DRAW_MODEL(next, RED)
			}
			for (next = groups.first; next; next = next->next) {
				DrawSphereWires(((struct group*)next->data)->position, ((struct group*)next->data)->radius, 24, 24, BLACK);
				current = ((struct group*)next->data)->group_selection.first;
				for (; current; current = current->next) {
					DRAW_MODEL(current, ORANGE)
				}
			}
			DrawSphereWires(camera.target, .3f, 12, 12, ORANGE);
			DrawSphere((Vector3){.0f, .0f, .0f}, 5.0f, BLUE);
			DrawCubeWires((Vector3){0.0f, 0.0f, -ARRIVAL_DIST / 2}, MAX_DIST*2, MAX_DIST*2, ARRIVAL_DIST, RED);
		EndMode3D();
		DrawText(TextFormat("Total de Modelos: %d", drawing.size + selection.size), 10, 30, 20, BLACK);
		DrawText("[Botao Esquerdo] selecionar\n"
			 "[Botao Direito] mover\n"
			 "[1 - 3] adicionar modelo de pedra\n"
			 "[4] adicionar modelo de nave inimiga\n"
			 "[CTRL  + C] copiar\n"
			 "[CTRL  + V] colar\n"
			 "[CTRL  + A] selecionar todos\n"
			 "[CTRL  + S] salvar\n"
			 "[CTRL  + O] abrir\n"
			 "[SHIFT + =] aumentar\n"
			 "[SHIFT + -] diminuir\n"
			 "[XYZ   + -] rotaticonar\n"
			 "[DEL] remover modelo", 10, 50, 20, BLACK);
		DrawFPS(10, 10);
		if (save_flag || open_flag) {
			DrawRectangle(10, 250, 600, 30, GRAY);
			DrawText(name, 10+3, 250, 30, WHITE);
		}
	EndDrawing();
}

void trans_single()
{
	if (move_flag)
		((struct model*)selected->data)->position = camera.target;
	if (IsKeyDown(KEY_X))
		((struct model*)selected->data)->angles.x += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_Y))
		((struct model*)selected->data)->angles.y += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_Z))
		((struct model*)selected->data)->angles.z += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_LEFT_SHIFT)) {
		if (IsKeyDown(KEY_EQUAL))
			((struct model*)selected->data)->scale += 1.0f;
		else if (IsKeyDown(KEY_MINUS))
			((struct model*)selected->data)->scale -= 1.0f;
	} else if (IsKeyDown(KEY_LEFT_CONTROL)) {
		if (IsKeyPressed(KEY_C)) {
			new_model.model = ((struct model*)selected->data)->model;
			new_model.angles = ((struct model*)selected->data)->angles;
			new_model.scale = ((struct model*)selected->data)->scale;
		} else if (IsKeyPressed(KEY_V)) {
			new_model.position = camera.target;
			list_insert(&new_model, &drawing, sizeof(struct model));
		}
	}
	if (IsKeyPressed(KEY_DELETE)) {
		list_remove(selected, &drawing);
		selected = NULL;
	}
}

#define MOVE_MANY(GROUP, PTR) \
		for (next = GROUP.first; next; next = next->next) { \
			((struct PTR*)next->data)->position.x += camera.target.x - lastpos.x; \
			((struct PTR*)next->data)->position.y += camera.target.y - lastpos.y; \
			((struct PTR*)next->data)->position.z += camera.target.z - lastpos.z; \
		}

void trans_selection()
{
	struct node *next;

	if (move_flag) {
		MOVE_MANY(selection, model)
		lastpos = camera.target;
	}
	if (IsKeyDown(KEY_X))
		for (next = selection.first; next; next = next->next)
			((struct model*)next->data)->angles.x += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_Y))
		for (next = selection.first; next; next = next->next)
			((struct model*)next->data)->angles.y += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_Z))
		for (next = selection.first; next; next = next->next)
			((struct model*)next->data)->angles.z += IsKeyDown(KEY_MINUS)? -.8f : .8f;
	if (IsKeyDown(KEY_LEFT_SHIFT)) {
		if (IsKeyDown(KEY_EQUAL))
			for (next = selection.first; next; next = next->next)
				((struct model*)next->data)->scale += 1.0f;
		else if (IsKeyDown(KEY_MINUS))
			for (next = selection.first; next; next = next->next)
				((struct model*)next->data)->scale -= 1.0f;
	} else if (IsKeyDown(KEY_LEFT_CONTROL)) {
		if (IsKeyPressed(KEY_C)) {
			for (next = selection.first; next; next = next->next) {
				new_model.model = ((struct model*)next->data)->model;
				new_model.angles = ((struct model*)next->data)->angles;
				new_model.scale = ((struct model*)next->data)->scale;
				new_model.position = ((struct model*)next->data)->position;
				list_insert(&new_model, &ln, sizeof(struct model));
			}
			lastpos = camera.target;
		} else if (IsKeyPressed(KEY_V)) {
			for (next = ln.first; next; next = next->next) {
				((struct model*)next->data)->position.x += camera.target.x - lastpos.x;
				((struct model*)next->data)->position.y += camera.target.y - lastpos.y;
				((struct model*)next->data)->position.z += camera.target.z - lastpos.z;
				list_insert(next->data, &drawing, sizeof(struct model));
			}
			while (ln.size)
				list_remove(ln.last, &ln);
		}
	}
	if (IsKeyPressed(KEY_DELETE))
		while (selection.size)
			list_remove(selection.last, &selection);
}

void get_in_volume(in_list, out_list, group_shape)
struct list *in_list, *out_list;
struct group *group_shape;
{
	struct node *next, *next_collision, *current;
	struct model *target, *ptrm;
	struct list *collisions;
	Vector3 temp;
	float new_scale;
	bool found;

	for (next = in_list->first; next;) {
		found = false;
		target = (struct model*)next->data;
		collisions = &models[target->model].collision_list;
		for (next_collision = collisions->first; next_collision; next_collision = next_collision->next) {
			ptrm = (struct model*)next_collision->data;
			temp = TRANFORM_SPHERE(target, ptrm->position)
			new_scale = ptrm->scale * target->scale;
			if (CheckCollisionSpheres(temp, new_scale, group_shape->position, group_shape->radius)) {
				list_insert(next->data, out_list, sizeof(struct model));
				current = next;
				next = next->next;
				list_remove(current, in_list);
				found = true;
				break;
			}
		}
		if (!found)
			next = next->next;
	}
}

#define RELEASE_GROUP_SELECTION(LIST) \
	while (LIST.first) { \
		list_insert(LIST.first->data, &drawing, sizeof(struct model)); \
		list_remove(LIST.first, &LIST); \
	}

#define ROTATE_GROUP(AXIS, DEGREE) \
	for (next = groups.first; next; next = next->next) { \
		current = ((struct group*)next->data)->group_selection.first; \
		for (; current; current = current->next) { \
			direction = Vector3RotateByAxisAngle( \
				Vector3Subtract(((struct model*)current->data)->position, \
						((struct group*)next->data)->position), \
				AXIS, \
				DEGREE); \
			((struct model*)current->data)->position = Vector3Add(direction, ((struct group*)next->data)->position); \
		} \
	}

void trans_group()
{
	struct node *next, *current;
	struct list *l;
	Vector3 direction;

	if (IsKeyPressed(KEY_B))
		for (next = groups.first; next; next = next->next)
			get_in_volume(&drawing, &(((struct group*)next->data)->group_selection), next->data);
	if (IsKeyPressed(KEY_V))
		for (next = groups.first; next; next = next->next) {
			l = &((struct group*)next->data)->group_selection;
			while (l->first) {
				list_insert(l->first->data, &selection, sizeof(struct model));
				list_remove(l->first, l);
			}
		}
	if (IsKeyPressed(KEY_R))
		for (next = groups.first; next; next = next->next)
			RELEASE_GROUP_SELECTION(((struct group*)next->data)->group_selection)
	if (IsKeyPressed(KEY_F))
		while (groups.first) {
			RELEASE_GROUP_SELECTION(((struct group*)groups.first->data)->group_selection)
			list_remove(groups.first, &groups);
		}
	if (move_flag) {
		MOVE_MANY(groups, group)
		for (current = groups.first; current; current = current->next)
			MOVE_MANY(((struct group*)current->data)->group_selection, model)
		lastpos = camera.target;
	}
	if (IsKeyDown(KEY_LEFT_SHIFT)) {
		if (IsKeyDown(KEY_EQUAL)) {
			for (next = groups.first; next; next = next->next) {
				((struct group*)next->data)->radius += 1.0f;
				current = ((struct group*)next->data)->group_selection.first;
				for (; current; current = current->next) {
					direction = Vector3Normalize(Vector3Subtract(((struct model*)current->data)->position, ((struct group*)next->data)->position));
					((struct model*)current->data)->position.x += direction.x;
					((struct model*)current->data)->position.y += direction.y;
					((struct model*)current->data)->position.z += direction.z;
				}
			}
		} else if (IsKeyDown(KEY_MINUS))
			for (next = groups.first; next; next = next->next) {
				((struct group*)next->data)->radius -= 1.0f;
				current = ((struct group*)next->data)->group_selection.first;
				for (; current; current = current->next) {
					direction = Vector3Normalize(Vector3Subtract(((struct group*)next->data)->position,
								                     ((struct model*)current->data)->position));
					((struct model*)current->data)->position.x += direction.x;
					((struct model*)current->data)->position.y += direction.y;
					((struct model*)current->data)->position.z += direction.z;
				}
			}
	}
	if (IsKeyDown(KEY_X))
		ROTATE_GROUP(((Vector3){1.0f, 0.0f, 0.0f}), IsKeyDown(KEY_MINUS)? -0.1f : 0.1f)
	if (IsKeyDown(KEY_Y))
		ROTATE_GROUP(((Vector3){0.0f, 1.0f, 0.0f}), IsKeyDown(KEY_MINUS)? -0.1f : 0.1f)
	if (IsKeyDown(KEY_Z))
		ROTATE_GROUP(((Vector3){0.0f, 0.0f, 1.0f}), IsKeyDown(KEY_MINUS)? -0.1f : 0.1f)
}
