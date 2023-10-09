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
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "defs.h"
#include "ai/neural_img.h"
#include "ui.c"
#include "spacecraft.c"

#define PLAY

/* TODO:
 * ranking;
 * municao;
 * separar os códigos de treinamento;
 * texturas;
 * sons.
 * */

struct enemy_spacecraft {
	struct model shape;
	float dist;
	struct list bullets;
	float last_shoot, last_meteor, last_shooted;
	Color color;
#ifdef PLAY
	int life;
#else
	float last_state[INPUT_QTT], last_move, last_far;
	struct {
		unsigned collision : 1;
		unsigned dont_shoot : 1;
		unsigned dont_move : 1;
		unsigned faraway : 1;
	} penalties;
	bool has_penalty;
#endif
};
#ifndef PLAY
struct enemy_shot {
	Vector3 position;
	float state[INPUT_QTT];
};

struct list enemy_errors = { 0 };
struct enemy_error {
	float x[INPUT_QTT], y[MAX_CLASSES];
} current_error;
#endif

struct models_with_collisions *models;
short model_count;
struct list rocks = { 0 };
struct list following = { 0 };
struct list shots = { 0 };
struct enemy_spacecraft new_enemy;

int screen_width, screen_height;
Camera camera;
Model mod_spaceship;
BoundingBox new_spaceship1, new_spaceship2;
void (*current_screen)();
bool exit_game = false;

#define WEIGHTS_LOCATION "data/weights"

main(argc, argv)
char *argv[];
{
	void load_map(), init_network(), menu(), game();

	SetRandomSeed(3);
	init_network();
	
	new_enemy.color = GRAY;
	new_enemy.last_shooted = 0.0f;
	new_enemy.last_meteor = new_enemy.last_shoot = .0f;
	new_enemy.bullets.last = new_enemy.bullets.first = NULL;
	new_enemy.bullets.size = 0;
#ifdef PLAY
	void load_scores(), save_scores();

	new_enemy.life = 5;
	game_state.life = MAX_LIFE;
#else
	int i;
	float now, prev_back = 0.0f;
	struct enemy_error *ptrerr;
	struct node *next;
	
	new_enemy.penalties.faraway = 0;	
	new_enemy.penalties.dont_move = 0;
	new_enemy.last_far = new_enemy.last_move = 0.0f;
	new_enemy.penalties.collision = 0;
	new_enemy.penalties.dont_shoot = 0;
	for (i=0; i < INPUT_QTT; i++)
		new_enemy.last_state[i] = 0;
	new_enemy.has_penalty = 0;
#endif

	camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
	camera.target = (Vector3){0.0f, 4.5f,-1.0f };
	camera.up = (Vector3){ .0f, 6.0f, .0f };
	camera.fovy = 90.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(0, 0, "Batalha Espacial");
	screen_width = GetScreenWidth();
	screen_height = GetScreenHeight();
#ifdef PLAY
	load_map("teste4.map");
	font = LoadFont("font/setback.png");
	load_scores();
	
	ToggleFullscreen();
	current_screen = menu;
#else
	load_map(argv[1]);
	current_screen = game;
#endif
	SetTargetFPS(60);

	mod_spaceship = LoadModel("models/spacecraft.glb");

#ifdef PLAY
	while (!WindowShouldClose() && !exit_game)
		current_screen();
#else
	while (!WindowShouldClose())
		game();
#endif
	CloseWindow();
#ifdef PLAY
	save_scores();
#else
	save_weights(WEIGHTS_LOCATION);
#endif
}

struct model enemies[10];
int total_enemies = 0, first_enemy = 0;
sort_by_dist(x, y)
void *x, *y;
{
	return ((struct model*)y)->position.z - ((struct model*)x)->position.z;
}

#ifdef PLAY
void load_scores()
{
	struct score_entry *new_score;
	unsigned char minutes, seconds, easy;
	char c, *ptrname;
	FILE *fp;
	
	ALLOCATE_LOCAL(scores_easy, sizeof(struct score_entry), 10)
	ALLOCATE_LOCAL(scores_hard, sizeof(struct score_entry), 10)
	if ((fp = fopen("data/scores.csv", "r"))) {
		for (;;) {
			if (fscanf(fp, "%hhd,", &easy) == EOF)
				break;
			if (easy) {
				new_score = (struct score_entry*) LAST_SPACE_LOCAL(scores_easy);
				scores_easy.nmemb++;
			} else {
				new_score = (struct score_entry*) LAST_SPACE_LOCAL(scores_hard);
				scores_hard.nmemb++;
			}
			fscanf(fp, "%d,", &new_score->score);
			for (ptrname = new_score->name; (c = fgetc(fp)) != ','; *ptrname++ = c);
			*ptrname = '\0';
			fscanf(fp, "%hhd/%hhd/%hhd,%hhd:%hhd\n", &new_score->day, &new_score->month, &new_score->year, &minutes, &seconds);
			new_score->seconds = seconds;
			new_score->seconds += minutes * 60;
		}
		fclose(fp);
	}
}

void save_scores()
{
	struct score_entry *new_score;
	FILE *fp;
	int i;

	if ((fp = fopen("data/scores.csv", "w"))) {
		for (i=0; i < scores_easy.nmemb; i++) {
			new_score = (struct score_entry*) AT(scores_easy, i);
			fprintf(fp, "%hhd,%d,%s,%hhd/%hhd/%hhd,%hhd:%hhd\n", 1, 
									     new_score->score,
									     new_score->name,
									     new_score->day,
									     new_score->month,
									     new_score->year,
									     new_score->seconds / 60,
									     new_score->seconds % 60);
		}
		for (i=0; i < scores_hard.nmemb; i++) {
			new_score = (struct score_entry*) AT(scores_hard, i);
			fprintf(fp, "%hhd,%d,%s,%hhd/%hhd/%hhd,%hhd:%hhd\n", 0, 
									     new_score->score,
									     new_score->name,
									     new_score->day,
									     new_score->month,
									     new_score->year,
									     new_score->seconds / 60,
									     new_score->seconds % 60);
		}
		fclose(fp);
	}
}

int screen = 0;

void menu()
{
	void game();

	BeginDrawing();
		ClearBackground(BLACK);
		DrawTextEx(font, "SPACECRAFT", (Vector2){
				screen_width/2 - MeasureTextEx(font, "SPACECRAFT", font.baseSize * TITLE_SIZE, SPACING).x/2,
				MeasureTextEx(font, "SPACECRAFT", font.baseSize * TITLE_SIZE, SPACING).y/4
				}, font.baseSize * TITLE_SIZE, SPACING, BLUE);
		switch (screen) {
			case 1:
				screen = draw_play(screen_width, screen_height);
				break;
			case 2:
				SetMousePosition(screen_width / 2, screen_height / 2);
				DisableCursor();

				camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
				camera.target = (Vector3){0.0f, 4.5f,-1.0f };
				camera.up = (Vector3){ .0f, 6.0f, .0f };
				game_state.life = MAX_LIFE;
				game_state.state = 0;
				game_state.prev_time = 0.0f;
				game_state.score = 0;
				while (following.first) {
					while (((struct enemy_spacecraft*)following.first->data)->bullets.first)
						list_remove(((struct enemy_spacecraft*)following.first->data)->bullets.first,
							    &((struct enemy_spacecraft*)following.first->data)->bullets);
					list_remove(following.first, &following);
				}
				first_enemy = 0;
				game_state.time = GetTime();
				
				current_screen = game;
				
				break;
			case 3:
				screen = draw_about(screen_width, screen_height);
				break;
			case 4:
				exit_game = true;
				break;
			default:
				screen = draw_menu(screen_width, screen_height);
				break;
		}
	EndDrawing();	
}
#endif

void game()
{
	void manage_player(), manage_enemies(), draw_scene();
#ifndef PLAY
	struct node *next;
	struct enemy_error *ptrerr;
	float now, prev_back;

	prev_back = 0.0f;
#endif
	UpdateMyCamera(&camera, GetFrameTime());
	manage_player();	
	manage_enemies();
#ifdef PLAY
	if (!game_state.state) {
		if (-camera.target.z > ARRIVAL_DIST)
			game_state.state = GetTime() - game_state.time >= DEADLINE_SECS-1.0f || following.size? -1 : 1;
		if (!game_state.life)
			game_state.state = -1;
	}
	game_state.distance = -camera.target.z;
#else
	now = GetTime();
	if (now - prev_back > 5.0f) {
		ini_backpr(enemy_errors.size);
		clear_backpr();
		for (next = enemy_errors.first; next; next = next->next) {
			ptrerr = (struct enemy_error*)next->data;
			backpr(ptrerr->x, ptrerr->y);
		}
		while (enemy_errors.first)
			list_remove(enemy_errors.first, &enemy_errors);
		apply_backpr();
		end_backpr();
		prev_back = GetTime();
	}
#endif
	BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode3D(camera);
			draw_scene();
		EndMode3D();
		DrawFPS(10, 10);
#ifdef PLAY
		if (draw_ui(screen_width, screen_height, &font)) {
			current_screen = menu;
			EnableCursor();
			screen = 1;
		}
#endif
	EndDrawing();
}

unsigned layers1[] = {INPUT_QTT, 20, 10, 5};
struct create_network nets[] = {{layers1, sizeof(layers1)/sizeof(unsigned), INPUT_QTT, 1, -1}};
void init_network()
{
	init_net_topology(nets, 1, 1);
	if (FileExists(WEIGHTS_LOCATION))
		load_weights(WEIGHTS_LOCATION, 1);
	else
		init_random_weights();
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
		if (enemy_idx != new.model)
			list_insert(&new, &rocks, sizeof(struct model));
		else
			enemies[total_enemies++] = new;
	}
	fclose(fp);
	qsort(enemies, total_enemies, sizeof(struct model), sort_by_dist);
}

void manage_player()
{
	static Matrix pos_spaceship;
        /* The minimum point is the front, left and bottom.
	 * There is a 90-degree rotation, counterclockwise and on the y-axis*/
	static BoundingBox box_spaceship1 = {
		{ .46f,-0.2f,-2.0f},
		{-.9f,	0.1f, 2.0f}
	};
	static BoundingBox box_spaceship2 = {
		{ 0.8f,0.1f,-0.5f},
		{-0.7f,0.8f, 0.5f}
	};
	static bool prev_collision = 0;
	static float prev_time = 0;

	struct node *curr, *next, *ptre;
	Vector3 *ptrbul, new_bullet;
	float now;
	bool collision;

	pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
					MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
			MatrixTranslate(camera.target.x, camera.target.y - 1.8f,
				camera.target.z + 0.5f));
	mod_spaceship.transform = pos_spaceship;
	new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
	new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
	new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
	new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
	/*player collision with meteors*/
	collision = collision_spacecraft_rocks();
	if (collision && !prev_collision) {
		prev_collision = true;
		game_state.life--;
	}
	if (!collision)
		prev_collision = false;
	/*player shots*/
	for (next = shots.first; next;) {
		ptrbul = (Vector3*)next->data;
		/*bullets collisions*/
		if (collision_bullet_rocks(ptrbul)) {
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
			continue;
		}
		now = GetTime();
#ifdef PLAY
		if (collision_bullet_enemies(ptrbul, &ptre)) {
			if (now - ((struct enemy_spacecraft*)ptre->data)->last_shooted > 0.5f) {
				((struct enemy_spacecraft*)ptre->data)->last_shooted = now;
				if (--((struct enemy_spacecraft*)ptre->data)->life == 0)
					list_remove(ptre, &following);
			}
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
			continue;
		}
#else
		if (collision_bullet_field(ptrbul, &ptre)) {
			if (now - ((struct enemy_spacecraft*)ptre->data)->last_shooted > 0.5f) {
				int i;
				
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[0] = GetRandomValue(0, 1);
				current_error.y[1] = !current_error.y[0];
				current_error.y[2] = GetRandomValue(0, 1);
				current_error.y[3] = !current_error.y[2];
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ((struct enemy_spacecraft*)ptre->data)->last_state[i];
				list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
			}
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
			((struct enemy_spacecraft*)ptre->data)->last_shooted = now;
			continue;
		}
#endif
		// bullets moves 
		if (ptrbul->z > -2 * ARRIVAL_DIST) {
			ptrbul->z -= BULLET_SPEED_PER_SECOND * GetFrameTime();
			next = next->next;
			continue;
		} else {
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
		}
	}
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
#ifdef PLAY
		game_state.score -= 2;
#endif
	}
}

collision_spacecraft_rocks()
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
			temp = TRANFORM_SPHERE(current, ptrm->position);
			if (CheckCollisionBoxSphere(new_spaceship1, temp, new_scale))
				return 1;
			if (CheckCollisionBoxSphere(new_spaceship2, temp, new_scale))
				return 1;
		}
	}
	return 0;
}

collision_bullet_rocks(bullet)
Vector3 *bullet;
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
			temp = TRANFORM_SPHERE(current, ptrm->position);
			if (CheckCollisionSpheres(*bullet, 1.0f, temp, new_scale))
				return 1;
		}
	}
	return 0;
}

#ifdef PLAY

#define BULLET_PLAYER_SIZE 1.0f

collision_bullet_enemies(bullet, enemy)
Vector3 *bullet;
struct node **enemy;
{
	struct node *next, *next_box;
	struct list *collisions_list;
	struct model *ptrm, *current;
	Vector3 temp;
	float new_scale;

	for (next = following.first; next; next = next->next) {
		current = &((struct enemy_spacecraft*)next->data)->shape; 
		collisions_list = &models[current->model].collision_list;
		for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
			ptrm = (struct model*)next_box->data;
			new_scale = ptrm->scale * current->scale;
			temp = TRANFORM_SPHERE(current, ptrm->position);
			if (CheckCollisionSpheres(*bullet, BULLET_PLAYER_SIZE, temp, new_scale)) {
				*enemy = next;
				return 1;
			}
		}
	}
	return 0;
}
#else

#define BULLET_PLAYER_SIZE 2.0f
#define ENEMY_FIELD 5.0f
#define LIMIT_DISTANCE_TO_PLAYER 10.0f

collision_bullet_field(bullet, enemy)
Vector3 *bullet;
struct node **enemy;
{
	struct node *next;
	struct model *current;

	for (next = following.first; next; next = next->next) {
		current = &((struct enemy_spacecraft*)next->data)->shape; 
		if (CheckCollisionSpheres(current->position, current->scale * ENEMY_FIELD, *bullet, BULLET_PLAYER_SIZE)) {
			*enemy = next;
			return 1;
		}
	}
	return 0;
}
#endif

void manage_enemies()
{
	struct node *next, *curr, *next_enemy;
	struct enemy_spacecraft *ptrenemy;
	float aux, now;
	float curr_state[INPUT_QTT];
	bool moved;
#ifdef PLAY
	Vector3 new_shot, *ptrbullet;
#else
	int i;
	struct enemy_shot new_shot, *ptrbullet;
#endif
	for (next_enemy = following.first; next_enemy;) {
		moved = false;
		ptrenemy = (struct enemy_spacecraft*)next_enemy->data;
#ifdef PLAY
		// enemy meteor collision
		now = GetTime();
		if (collision_enemy_rocks(ptrenemy) && now - ptrenemy->last_meteor > 0.5f) {
			if (--ptrenemy->life == 0) {
				curr = next_enemy;
				next_enemy = next_enemy->next;
				list_remove(curr, &following);
				continue;
			}
			ptrenemy->last_meteor = now;
		}
		for (next = ptrenemy->bullets.first; next;) {
			ptrbullet = (Vector3*)next->data;
			// enemy bullet collision
			if (collision_bullet_rocks(ptrbullet)) {
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			if (collision_bullet_player(ptrbullet)) {
				--game_state.life;
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			// bullet move
			if (ptrbullet->z < MAX_DIST) {
				ptrbullet->z += BULLET_SPEED_PER_SECOND * GetFrameTime();
				next = next->next;
				continue;
			} else {
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
			}
		}
#else
		now = GetTime();
		if (Vector3Distance(ptrenemy->shape.position,
					(Vector3){camera.target.x,
					camera.target.y - 1.8f, camera.target.z
					+ 0.5f}) > ptrenemy->dist + LIMIT_DISTANCE_TO_PLAYER &&
		    now - ptrenemy->last_far > 2.0f) {
			ptrenemy->penalties.faraway = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->color = RED;
			ptrenemy->last_far = now;
		}
		now = GetTime();
		// enemy meteor collision
		if (collision_enemy_rocks(&ptrenemy->shape.position, ptrenemy->shape.scale * ENEMY_FIELD) &&
		    now - ptrenemy->last_meteor > 1.0f) {
			ptrenemy->penalties.collision = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->color = RED;
			ptrenemy->last_meteor = now;
		} else
			ptrenemy->color = GRAY;
		for (next = ptrenemy->bullets.first; next;) {
			ptrbullet = (struct enemy_shot*)next->data;
			// enemy bullet collision
			if (collision_bullet_rocks(ptrbullet->position)) {
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[GetRandomValue(0,3)] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ptrbullet->state[i];
				list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			if (collision_bullet_player(ptrbullet->position)) {
				--game_state.life;
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[4] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ptrbullet->state[i];
				list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			// bullet move
			if (ptrbullet->position.z < MAX_DIST) {
				ptrbullet->position.z += BULLET_SPEED_PER_SECOND * GetFrameTime();
				next = next->next;
				continue;
			} else {
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[GetRandomValue(0,3)] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ptrbullet->state[i];
				list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
			}
		}
#endif
		 next_enemy = next_enemy->next;
	}
#ifndef PLAY
	for (next = following.first; next; next = next->next) {
		ptrenemy = (struct enemy_spacecraft*)next->data;
		if (ptrenemy->has_penalty) {
			for (i=0; i < MAX_CLASSES; i++)
				current_error.y[i] = 0.0f;
			current_error.y[GetRandomValue(0,3)] = 1.0f;
			if (ptrenemy->penalties.dont_shoot)
				current_error.y[4] = 1.0f;
			for (i=0; i < INPUT_QTT; i++)
				current_error.x[i] = ptrenemy->last_state[i];
			list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
			ptrenemy->penalties.collision = 0; 
			ptrenemy->penalties.dont_shoot = 0;
			ptrenemy->penalties.dont_move = 0;
			ptrenemy->penalties.faraway = 0;
		}
	}
#endif
	/*curr_state[0-2] está normalizado entre -1 e 1*/
	/*curr_state[3-6] está normalizado entre 0 e 1*/
	for (next = following.first; next; next = next->next) {
		ptrenemy = (struct enemy_spacecraft*)next->data;
		curr_state[6] = curr_state[5] = curr_state[4] = DIAGONAL_MAP;
		for (curr = rocks.first; curr; curr = curr->next) {
			aux = Vector3Distance(ptrenemy->shape.position,
					      ((struct model*)curr->data)->position);
			if (curr_state[4] > aux) {
				curr_state[6] = curr_state[5];
				curr_state[5] = curr_state[4];
				curr_state[4] = aux;
			} else if (curr_state[5] > aux) {
				curr_state[6] = curr_state[5];
				curr_state[5] = aux;
			} else if (curr_state[6] > aux)
				curr_state[6] = aux;
		}
		curr_state[4] /= DIAGONAL_MAP;
		curr_state[5] /= DIAGONAL_MAP;
		curr_state[6] /= DIAGONAL_MAP;
		curr_state[0] = ptrenemy->shape.position.x / MAX_DIST;
		curr_state[1] = ptrenemy->shape.position.y / MAX_DIST;
		curr_state[2] = (ARRIVAL_DIST / 2 + ptrenemy->shape.position.z) / (-ARRIVAL_DIST / 2);
		curr_state[7] = camera.target.x / MAX_DIST;
		curr_state[8] = (camera.target.y - 1.8f) / MAX_DIST;
		curr_state[9] = (ARRIVAL_DIST / 2 + camera.target.z + 0.5f) / (-ARRIVAL_DIST / 2);
		curr_state[3] = Vector3Distance(ptrenemy->shape.position,
					       (Vector3){camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f}) / DIAGONAL_MAP;
		curr_state[10] = DIAGONAL_MAP;
		for (curr = shots.first; curr; curr = curr->next) {
			aux = Vector3Distance(ptrenemy->shape.position, *(Vector3*)curr->data);
			if (curr_state[10] > aux)
				curr_state[10] = aux;
		}
		curr_state[10] /= DIAGONAL_MAP;
		run(curr_state);
#ifndef PLAY
		for (i=0; i < INPUT_QTT; i++)
			ptrenemy->last_state[i] = curr_state[i];
		ptrenemy->has_penalty = 0;
		if (isnan(network_output[0]))
			exit(0);
#endif
		if (network_output[0] > .3f) {
			ptrenemy->shape.position.x += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x >  MAX_DIST-10.0f)
				ptrenemy->shape.position.x =  MAX_DIST-10.0f;
			else
				moved = true;
		}
		if (network_output[1] > .3f) {
			ptrenemy->shape.position.x -= VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x < -MAX_DIST+10.0f)
				ptrenemy->shape.position.x = -MAX_DIST+10.0f;
			else
				moved = true;
		}
		if (network_output[2] > .3f) {
			ptrenemy->shape.position.y += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.y >  MAX_DIST-10.0f)
				ptrenemy->shape.position.y =  MAX_DIST-10.0f;
			else
				moved = true;
		}
		if (network_output[3] > .3f) {
			ptrenemy->shape.position.y -= VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.y <  -MAX_DIST+10.0f)
				ptrenemy->shape.position.y =  -MAX_DIST+10.0f;
			else
				moved = true;
		}
		/*shot*/
		now = GetTime();
#ifdef PLAY
		if (network_output[4] > .3f) {
			if (now - ptrenemy->last_shoot > 1.0f / FIRING_RATE_PER_SECOND)	{
				ptrenemy->last_shoot = now;
				new_shot = ptrenemy->shape.position;
				new_shot.z += 2.0f;
				list_insert(&new_shot, &ptrenemy->bullets, sizeof(Vector3));
			}
		}
#else
		if (now - ptrenemy->last_move > 2.0f && !moved) {
			ptrenemy->penalties.dont_move = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_move = now;
		}
		if (network_output[4] > .3f) {
			if (now - ptrenemy->last_shoot > 1.0f / FIRING_RATE_PER_SECOND)	{
				ptrenemy->last_shoot = now;
				new_shot.position = ptrenemy->shape.position;
				new_shot.position.z += 2.0f;
				for (i=0; i < INPUT_QTT; i++)
					new_shot.state[i] = ptrenemy->last_state[i];	
				list_insert(&new_shot, &ptrenemy->bullets, sizeof(struct enemy_shot));
			}
		} else if (now - ptrenemy->last_shoot > 1.0f) {
			ptrenemy->penalties.dont_shoot = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_shoot = now;
		}
#endif
	}
}

void draw_scene()
{
	struct node *next, *curr;
	struct model *draw_model;
	Vector3 position, new_bullet, aux_bullet;
	float inc, limit, dist;
	int i;

	for (next = rocks.first; next; next = next->next) {
		draw_model = (struct model*)next->data;
		DrawModelRotate(models[draw_model->model].drawing,
				draw_model->position,
				draw_model->angles,
				draw_model->scale,
				GRAY);
#ifndef PLAY
		draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);
#endif
	}
	for (i=first_enemy; i < total_enemies; i++) {
		draw_model = &enemies[i];
		if (i)
			if ((dist = enemies[i-1].position.z - draw_model->position.z) > 40.0f)
				limit = DISTANCE_FROM_PLAYER;
			else
				limit = DISTANCE_FROM_PLAYER + dist;
		else
			limit = DISTANCE_FROM_PLAYER;
		if (draw_model->position.z + limit - camera.target.z >= .0f) {
			new_enemy.shape = enemies[i];
			new_enemy.dist = camera.target.z - enemies[i].position.z;
			list_insert(&new_enemy, &following, sizeof(struct enemy_spacecraft));
			first_enemy++;
			continue;
		}
		/*enemy spawn -- arrival*/
		inc = INITIAL_ENEMY_DIST * fabs(draw_model->position.z + limit - camera.target.z) /
							  (-draw_model->position.z);
		position = draw_model->position;
		position.x = position.x > .0f? position.x + inc : position.x - inc;
		position.y = position.y > .0f? position.y + inc : position.y - inc;
		DrawModelRotate(models[draw_model->model].drawing,
				position,
				draw_model->angles,
				draw_model->scale,
				GRAY);
	}
	for (next = following.first; next; next = next->next) {
		draw_model = &((struct enemy_spacecraft*)next->data)->shape;
		draw_model->position.z -= ((struct enemy_spacecraft*)next->data)->dist - fabs(camera.target.z - draw_model->position.z);
		DrawModelRotate(models[draw_model->model].drawing,
				draw_model->position,
				draw_model->angles,
				draw_model->scale,
				((struct enemy_spacecraft*)next->data)->color);
#ifndef PLAY
		draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);
		DrawSphereWires((Vector3){draw_model->position.x,
				draw_model->position.y,
				draw_model->position.z},
				draw_model->scale * ENEMY_FIELD,
				4, 4,
				BLACK);
		DrawLine3D(draw_model->position,
			  (Vector3){camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f}, 
			  Vector3Distance(draw_model->position, 
				          (Vector3){camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f}) >
			                  ((struct enemy_spacecraft*)next->data)->dist + LIMIT_DISTANCE_TO_PLAYER? RED : BLUE);
#endif
	}
	DrawSphere((Vector3){camera.position.x+250.0f, camera.position.y+150.0f, camera.position.z-700.0f}, 100.0f, YELLOW);
	DrawModel(mod_spaceship, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
	for (next = shots.first; next; next = next->next) {
		aux_bullet = new_bullet = *(Vector3*)next->data;
		new_bullet.z += .2f;
		aux_bullet.z -= .4f;
		DrawCapsule(new_bullet, aux_bullet, .125f, 4, 4, ORANGE);
	}
	for (curr = following.first; curr; curr = curr->next)
		for (next = ((struct enemy_spacecraft*)curr->data)->bullets.first; next; next = next->next)
			DrawSphere(*(Vector3*)next->data, ENEMY_BULLET_SIZE, RED);
}
#ifdef PLAY
collision_enemy_rocks(enemy)
struct enemy_spacecraft *enemy;
{
	struct node *next, *next_box, *next_enemy_box;
	struct list *collisions_list;
	struct model *ptrm, *current, *eptrm, *ecurrent;
	Vector3 temp, etemp;
	float new_scale, enew_scale;
	
	ecurrent = &enemy->shape;
	for (next = rocks.first; next; next = next->next) {
		current = (struct model*)next->data;
		collisions_list = &models[current->model].collision_list;
		for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
			ptrm = (struct model*)next_box->data;
			new_scale = ptrm->scale * current->scale;
			temp = TRANFORM_SPHERE(current, ptrm->position);
			for (next_enemy_box = models[enemy->shape.model].collision_list.first; next_enemy_box; next_enemy_box = next_enemy_box->next) {
				eptrm = (struct model*)next_box->data;
				enew_scale = eptrm->scale * enemy->shape.scale;
				etemp = TRANFORM_SPHERE(ecurrent, eptrm->position);
				if (CheckCollisionSpheres(temp, new_scale, etemp, enew_scale))
					return 1;
			}
		}
	}
	return 0;
}
#else
collision_enemy_rocks(enemy_pos, enemy_range)
Vector3 *enemy_pos;
float enemy_range;
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
			temp = TRANFORM_SPHERE(current, ptrm->position);
			if (CheckCollisionSpheres(temp, new_scale, *enemy_pos, enemy_range))
				return 1;
		}
	}
	return 0;
}
#endif

collision_bullet_player(bullet)
Vector3 *bullet;
{

	if (CheckCollisionBoxSphere(new_spaceship1, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	if (CheckCollisionBoxSphere(new_spaceship2, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	return 0;
}
