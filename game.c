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
 * texturas;
 * municao.
 * */

struct enemy_spacecraft {
	struct model shape;
	float dist;
	struct list bullets;
	float last_shoot, last_meteor_penalty, last_meteor, last_shooted, last_shooted_penalty;
	Color color;
	int life;
	float last_state[INPUT_QTT], last_move, last_far;
	struct {
		unsigned collision : 1;
		unsigned dont_shoot : 1;
		unsigned dont_move : 1;
		unsigned faraway : 1;
	} penalties;
	bool has_penalty;
};

struct enemy_shot {
	Vector3 position;
	float state[INPUT_QTT];
};

struct list enemy_errors = { 0 };
struct enemy_error {
	float x[INPUT_QTT], y[MAX_CLASSES];
} current_error;

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

struct sound_I {
	Sound sound;
	float P;
} destroyed_sound, enemy_shot_sound, \
  hit_rock_player_shot, hit_rock_enemy_shot,
  hit_enemy, hit_rock_enemy_spacecraft;

Sound shot_sound, victory_sound, defeat_sound, hit_player, collision_sound;

#define WEIGHTS_LOCATION "data/weights"

main(argc, argv)
char *argv[];
{
	void load_map(), init_network(), menu(), game();
	int i;
#ifdef PLAY
	void load_scores(), save_scores();

	new_enemy.life = ENEMY_LIFE;
	game_state.life = MAX_LIFE;
#endif
	new_enemy.color = GRAY;
	new_enemy.last_shooted = 0.0f;
	new_enemy.last_meteor_penalty = new_enemy.last_shooted_penalty = 0.0f;
	new_enemy.last_meteor = new_enemy.last_shoot = 0.0f;
	new_enemy.bullets.last = new_enemy.bullets.first = NULL;
	new_enemy.bullets.size = 0;
	new_enemy.penalties.faraway = 0;	
	new_enemy.penalties.dont_move = 0;
	new_enemy.last_far = new_enemy.last_move = 0.0f;
	new_enemy.penalties.collision = 0;
	new_enemy.penalties.dont_shoot = 0;
	for (i=0; i < INPUT_QTT; i++)
		new_enemy.last_state[i] = 0;
	new_enemy.has_penalty = 0;
	
	SetRandomSeed(3);
	init_network();

	camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
	camera.target = (Vector3){0.0f, 4.5f,-1.0f };
	camera.up = (Vector3){ .0f, 6.0f, .0f };
	camera.fovy = 90.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(0, 0, "Batalha Espacial");
	screen_width = GetScreenWidth();
	screen_height = GetScreenHeight();
	
	InitAudioDevice();
	ui_sound = LoadSound("sounds/ui.wav");
	SetSoundVolume(ui_sound, 2.0f);
	shot_sound = LoadSound("sounds/laser3.mp3");
	SetSoundVolume(shot_sound, 0.08f);
	collision_sound = LoadSound("sounds/hit-rock-spacecraft.wav");
	SetSoundVolume(collision_sound, 0.2f);
	victory_sound = LoadSound("sounds/win.mp3");
	defeat_sound = LoadSound("sounds/lost.mp3");
	SetSoundVolume(defeat_sound, 0.5f);
	hit_player = LoadSound("sounds/hit-player.wav");
	SetSoundVolume(hit_player, 0.2f);
	
	hit_enemy.sound = LoadSound("sounds/hit-enemy.mp3");
	hit_enemy.P = 300.0f;
	hit_rock_player_shot.sound = LoadSound("sounds/hit-rock-player-shot.mp3");
	hit_rock_player_shot.P = 300.0f;
	hit_rock_enemy_shot.sound = LoadSound("sounds/hit-rock-enemy-shot.mp3");
	hit_rock_enemy_shot.P = 150.0f;
	hit_rock_enemy_spacecraft.sound = LoadSound("sounds/hit-rock-enemy-spacecraft.wav");
	hit_rock_enemy_spacecraft.P = 24.0f;
	enemy_shot_sound.sound = LoadSound("sounds/enemy-shot.mp3");
	enemy_shot_sound.P = 24.0f;
	destroyed_sound.sound = LoadSound("sounds/enemy-destroyed.mp3");
	destroyed_sound.P = 24.0f;
#ifdef PLAY
	load_map("teste4.map");
	font = LoadFont("font/setback.png");
	load_scores();
	ToggleFullscreen();
	current_screen = menu;
#else
	load_map(argv[1]);
	DisableCursor();
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
	UnloadSound(hit_rock_enemy_spacecraft.sound);
	UnloadSound(hit_rock_enemy_shot.sound);
	UnloadSound(hit_rock_player_shot.sound);
	UnloadSound(hit_player);
	UnloadSound(hit_enemy.sound);
	UnloadSound(shot_sound);
	UnloadSound(enemy_shot_sound.sound);
	UnloadSound(destroyed_sound.sound);
	UnloadSound(collision_sound);
	UnloadSound(victory_sound);
	UnloadSound(defeat_sound);
	UnloadSound(ui_sound);
	CloseAudioDevice();	
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
				game_state.score = total_enemies * ENEMY_LIFE * SCORE_PER_SHOT * 2;
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
	struct node *next;
	struct enemy_error *ptrerr;
	float now, prev_back;

	prev_back = 0.0f;
	UpdateMyCamera(&camera, GetFrameTime());
	manage_player();	
	manage_enemies();
#ifdef PLAY
	if (!game_state.state) {
		if (GetTime() - game_state.time >= DEADLINE_SECS-1.0f)
			game_state.state = -1;
		if (-camera.target.z > ARRIVAL_DIST)
			game_state.state = following.size? -1 : 1;
		else if (!game_state.life)
			game_state.state = -1;
		if (game_state.state == -1)
			PlaySound(defeat_sound);
		else if (game_state.state == 1)
			PlaySound(victory_sound);
	}
	game_state.distance = -camera.target.z;
#endif
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

#define SPACESHIP_POS camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f
#define PLAY_SOUND_BY_DIST(POINT, SOUND) \
			float I = Vector3Distance(POINT, (Vector3){SPACESHIP_POS}); \
			I *= 2; \
			I = SOUND.P / I; \
			SetSoundVolume(SOUND.sound, I); \
			PlaySound(SOUND.sound);
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
	int i;
	bool collision;

	pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
					MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
			MatrixTranslate(SPACESHIP_POS));
	mod_spaceship.transform = pos_spaceship;
	new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
	new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
	new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
	new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
	/*player collision with meteors*/
	collision = collision_spacecraft_rocks();
	if (collision && !prev_collision) {
		PlaySound(collision_sound);
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
			PLAY_SOUND_BY_DIST(*ptrbul, hit_rock_player_shot)
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
			continue;
		}
		now = GetTime();
		// penalty
		if (collision_bullet_field(ptrbul, &ptre)) {
			if (now - ((struct enemy_spacecraft*)ptre->data)->last_shooted_penalty > 0.5f) {
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[GetRandomValue(0,3)] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ((struct enemy_spacecraft*)ptre->data)->last_state[i];
				list_insert(&current_error, &enemy_errors, sizeof(struct enemy_error));
			}
			((struct enemy_spacecraft*)ptre->data)->last_shooted_penalty = now;
		}
		if (collision_bullet_enemies(ptrbul, &ptre)) {
			PLAY_SOUND_BY_DIST(*ptrbul, hit_enemy)
			((struct enemy_spacecraft*)ptre->data)->last_shooted = now;
			if (--((struct enemy_spacecraft*)ptre->data)->life == 0) {
				PLAY_SOUND_BY_DIST(((struct enemy_spacecraft*)ptre->data)->shape.position, destroyed_sound)	
				list_remove(ptre, &following);
			}
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
			continue;
		}
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
		game_state.score -= SCORE_PER_SHOT;
		PlaySound(shot_sound);
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

void manage_enemies()
{
	struct node *next, *curr, *next_enemy;
	struct enemy_spacecraft *ptrenemy;
	float aux, now;
	float curr_state[INPUT_QTT];
	bool moved;
	int i;
	struct enemy_shot new_shot, *ptrbullet;
	
	for (next_enemy = following.first; next_enemy;) {
		moved = false;
		ptrenemy = (struct enemy_spacecraft*)next_enemy->data;
		now = GetTime();
		// penalty 
		if (collision_enemy_field_rocks(&ptrenemy->shape.position, ptrenemy->shape.scale * ENEMY_FIELD) &&
		    now - ptrenemy->last_meteor_penalty > 1.0f) {
			ptrenemy->penalties.collision = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_meteor_penalty = now;
		}
		// enemy meteor collision
		if (collision_enemy_rocks(ptrenemy) && now - ptrenemy->last_meteor > 0.5f) {
			PLAY_SOUND_BY_DIST(ptrenemy->shape.position, hit_rock_enemy_spacecraft)
			if (--ptrenemy->life == 0) {
				PLAY_SOUND_BY_DIST(ptrenemy->shape.position, destroyed_sound)
				curr = next_enemy;
				next_enemy = next_enemy->next;
				list_remove(curr, &following);
				continue;
			}
			ptrenemy->last_meteor = now;
		}
		// enemy away from player or in the corners
		now = GetTime();
		if ((Vector3Distance(ptrenemy->shape.position,
				   (Vector3){camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f}
				   ) > ptrenemy->dist + LIMIT_DISTANCE_TO_PLAYER ||
		    fabs(ptrenemy->shape.position.x) > CORNER ||
		    fabs(ptrenemy->shape.position.y > CORNER)
		    ) && now - ptrenemy->last_far > 2.0f) {
			ptrenemy->penalties.faraway = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_far = now;
		}

		for (next = ptrenemy->bullets.first; next;) {
			ptrbullet = (struct enemy_shot*)next->data;
			// enemy bullet collision
			if (collision_bullet_rocks(&ptrbullet->position)) {
				PLAY_SOUND_BY_DIST(ptrbullet->position, hit_rock_enemy_shot)
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
			// hit the player
			if (collision_bullet_player(&ptrbullet->position)) {
#ifdef PLAY
				PlaySound(hit_player);
				--game_state.life;
#endif
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
				// bullet crossed the map
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
		next_enemy = next_enemy->next;
	}
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
		for (i=0; i < INPUT_QTT; i++)
			ptrenemy->last_state[i] = curr_state[i];
		ptrenemy->has_penalty = 0;
		
		if (isnan(network_output[0])) {
#ifdef PLAY
			current_screen = menu;
			return;
#else
			exit(0);
#endif
		}

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
				PLAY_SOUND_BY_DIST(new_shot.position, enemy_shot_sound);
			}
		} else if (now - ptrenemy->last_shoot > 1.0f) {
			ptrenemy->penalties.dont_shoot = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_shoot = now;
		}
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

collision_enemy_field_rocks(enemy_pos, enemy_range)
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

collision_bullet_player(bullet)
Vector3 *bullet;
{

	if (CheckCollisionBoxSphere(new_spaceship1, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	if (CheckCollisionBoxSphere(new_spaceship2, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	return 0;
}
