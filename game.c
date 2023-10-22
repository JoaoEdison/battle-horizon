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
#define RLIGHTS_IMPLEMENTATION
#include "shaders/rlights.h"

#define PLAY

/* TODO:
 * change the name of the game...
 * ammunition;
 * fix errors.
 * */

struct enemy_spacecraft {
	struct model shape;
	float dist;
	unsigned char head;
	float last_shoot, last_asteroid_penalty, last_asteroid, last_shooted_penalty;
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

struct shot {
	Vector3 position;
	short light_idx;
};

struct enemy_shot {
	struct shot properties;
	float state[INPUT_QTT];
	unsigned char head;
};

#define WEIGHTS_LOCATION "data/weights"

bignet_ptr three_heads[HEAD_COUNT];

struct list enemy_errors[HEAD_COUNT] = { 0 };
struct enemy_error {
	float x[INPUT_QTT], y[MAX_CLASSES];
} current_error;

struct models_with_collisions *models;
short model_count;
struct list asteroids = { 0 };
struct list following = { 0 };
struct list shots = { 0 };
struct list enemy_bullets = { 0 };
struct enemy_spacecraft new_enemy;

int screen_width, screen_height;
Camera camera;
Model mod_spaceship, mod_skybox;
BoundingBox new_spaceship1, new_spaceship2;
void (*current_screen)();
bool exit_game = false;

struct sound_I {
	Sound sound;
	float P;
} destroyed_sound, enemy_shot_sound,
  hit_asteroid_player_shot, hit_asteroid_enemy_shot,
  hit_enemy, hit_asteroid_enemy_spacecraft;

Sound shot_sound, victory_sound, defeat_sound, hit_player, collision_sound;

Shader shader, shader_cubemap;
Light lights[MAX_LIGHTS];

#define MODEL_COLOR WHITE

#define SUN_X   380.0f
#define SUN_Y   320.0f
#define SUN_Z (700.0f)

main(argc, argv)
char *argv[];
{
	void load_map(), unload_map(), init_network(), menu(), game(), load_skybox();
	int i;
#ifdef PLAY
	void load_scores(), save_scores();

	new_enemy.life = ENEMY_LIFE;
	game_state.life = MAX_LIFE;
#endif
	new_enemy.color = MODEL_COLOR;
	new_enemy.last_asteroid_penalty = new_enemy.last_shooted_penalty = 0.0f;
	new_enemy.last_asteroid = new_enemy.last_shoot = 0.0f;
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
	InitWindow(0, 0, t_t_game);
	screen_width = GetScreenWidth();
	screen_height = GetScreenHeight();
	
	InitAudioDevice();
	ui_sound = LoadSound("sounds/ui.wav");
	SetSoundVolume(ui_sound, 2.0f);
	shot_sound = LoadSound("sounds/laser3.mp3");
	SetSoundVolume(shot_sound, 0.08f);
	collision_sound = LoadSound("sounds/hit-asteroid-spacecraft.wav");
	SetSoundVolume(collision_sound, 0.2f);
	victory_sound = LoadSound("sounds/win.mp3");
	defeat_sound = LoadSound("sounds/lost.mp3");
	SetSoundVolume(defeat_sound, 0.5f);
	hit_player = LoadSound("sounds/hit-player.wav");
	SetSoundVolume(hit_player, 0.1f);
	
	hit_enemy.sound = LoadSound("sounds/hit-enemy.mp3");
	hit_enemy.P = 150.0f;
	hit_asteroid_player_shot.sound = LoadSound("sounds/hit-asteroid-player-shot.mp3");
	hit_asteroid_player_shot.P = 150.0f;
	hit_asteroid_enemy_shot.sound = LoadSound("sounds/hit-asteroid-enemy-shot.mp3");
	hit_asteroid_enemy_shot.P = 75.0f;
	hit_asteroid_enemy_spacecraft.sound = LoadSound("sounds/hit-asteroid-enemy-spacecraft.wav");
	hit_asteroid_enemy_spacecraft.P = 12.0f;
	enemy_shot_sound.sound = LoadSound("sounds/enemy-shot.mp3");
	enemy_shot_sound.P = 12.0f;
	destroyed_sound.sound = LoadSound("sounds/enemy-destroyed.mp3");
	destroyed_sound.P = 12.0f;
	
	load_skybox();	
	mod_spaceship = LoadModel("models/spacecraft.glb");
	//shaders
	shader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	//int ambientLoc = GetShaderLocation(shader, "ambient");
	//SetShaderValue(shader, ambientLoc, (float[4]){0.1f, 0.1f, 0.1f, 1.0f}, SHADER_UNIFORM_VEC4);
	for (i=0; i < mod_spaceship.materialCount; i++)
		mod_spaceship.materials[i].shader = shader;
	// create directional light facing 0,0,0	
	CreateLight(LIGHT_DIRECTIONAL, (Vector3){SUN_X, SUN_Y, SUN_Z}, Vector3Zero(), WHITE, shader);
	// create all possible lights in sun
	for (i=1; i < MAX_LIGHTS; i++)
		lights[i] = CreateLight(LIGHT_POINT, (Vector3){SUN_X, SUN_Y, SUN_Z}, Vector3Zero(), WHITE, shader);
	SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], (float[3]){SUN_X, SUN_Y, SUN_Z}, SHADER_UNIFORM_VEC3);

	SetTargetFPS(60);
#ifdef PLAY
	font = LoadFont("font/setback.png");
	load_scores();
	ToggleFullscreen();
	current_screen = menu;
#else
	load_map(argv[1]);
	DisableCursor();
	current_screen = game;
#endif

#ifdef PLAY
	while (!WindowShouldClose() && !exit_game)
		current_screen();
#else
	while (!WindowShouldClose())
		game();
#endif
	CloseWindow();
	UnloadSound(hit_asteroid_enemy_spacecraft.sound);
	UnloadSound(hit_asteroid_enemy_shot.sound);
	UnloadSound(hit_asteroid_player_shot.sound);
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

#define MOVE_LIGHT_SUN(IDX) \
		lights[IDX].position = (Vector3){SUN_X, SUN_Y, SUN_Z}; \
		lights[IDX].color = WHITE; \
		UpdateLightValues(shader, lights[IDX]);

#ifdef PLAY
void load_scores()
{
	struct score_entry *new_score;
	unsigned char minutes, seconds, easy;
	char c, *ptrname;
	FILE *fp;
	
	ALLOCATE(scores_easy, sizeof(struct score_entry), 10)
	ALLOCATE(scores_hard, sizeof(struct score_entry), 10)
	if ((fp = fopen("data/scores.csv", "r"))) {
		for (;;) {
			if (fscanf(fp, "%hhd,", &easy) == EOF)
				break;
			if (easy) {
				new_score = (struct score_entry*) LAST_SPACE(scores_easy);
				scores_easy.nmemb++;
			} else {
				new_score = (struct score_entry*) LAST_SPACE(scores_hard);
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
	void game(), load_map();

	BeginDrawing();
		ClearBackground(BLACK);
		DrawTextEx(font, t_t_game, (Vector2){
				screen_width/2 - MeasureTextEx(font, t_t_game, font.baseSize * TITLE_SIZE, SPACING).x/2,
				MeasureTextEx(font, t_t_game, font.baseSize * TITLE_SIZE, SPACING).y/4
				}, font.baseSize * TITLE_SIZE, SPACING, BLUE);
		switch (screen) {
			case 1:
				screen = draw_play(screen_width, screen_height);
				break;
			case 2:
				load_map(easy_map? "data/easy.map" : "data/hard.map");
				SetMousePosition(screen_width / 2, screen_height / 2);
				DisableCursor();

				camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
				camera.target = (Vector3){0.0f, 4.5f,-1.0f };
				camera.up = (Vector3){ .0f, 6.0f, .0f };
				game_state.life = MAX_LIFE;
				game_state.state = 0;
				game_state.prev_time = 0.0f;
				game_state.score = total_enemies * ENEMY_LIFE * SCORE_PER_SHOT * 2;
				while (following.first)
					list_remove(following.first, &following);
				while (enemy_bullets.first) {
					MOVE_LIGHT_SUN(((struct enemy_shot *)enemy_bullets.first->data)->properties.light_idx)
					list_remove(enemy_bullets.first, &enemy_bullets);
				}
				while (shots.first) {
					MOVE_LIGHT_SUN(((struct shot *)shots.first->data)->light_idx)
					list_remove(shots.first, &shots);
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
	int i;

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
		for (i=0; i < HEAD_COUNT; i++) {
			three_heads[i]->N = enemy_errors[i].size;
			clear_backpr(three_heads[i]);
			for (next = enemy_errors[i].first; next; next = next->next) {
				ptrerr = (struct enemy_error*)next->data;
				backpr(three_heads[i], ptrerr->x, ptrerr->y);
			}
			while (enemy_errors[i].first)
				list_remove(enemy_errors[i].first, &enemy_errors[i]);
			apply_backpr(three_heads[i]);
		}
		prev_back = GetTime();
	}
	BeginDrawing();
#ifdef PLAY
		ClearBackground(BLACK);
#else
		ClearBackground(WHITE);
#endif
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

unsigned layers1[] = {INPUT_QTT, 20, 10, MAX_CLASSES};
create_network_arr nets = {{layers1, sizeof(layers1)/sizeof(unsigned), INPUT_QTT, 1, -1}};

void init_network()
{
	int i;
	
	if (FileExists(WEIGHTS_LOCATION))
		for (i=0; i < HEAD_COUNT; i++) {
			three_heads[i] = load_weights(WEIGHTS_LOCATION, 1);
			ini_backpr(three_heads[i], 2);
		}
	else
		for (i=0; i < HEAD_COUNT; i++) {
			three_heads[i] = init_net_topology(nets, 1, 1);
			init_random_weights(three_heads[i]);
			ini_backpr(three_heads[i], 2);
		}
}

void load_map(name)
char name[];
{
	struct model new;
	short i, j, k, enemy_idx;
	FILE *fp;
	
	enemy_idx = -1;
	fp = fopen(name, "r");
	fscanf(fp, "%hd\n", &model_count);
	models = malloc(sizeof(struct models_with_collisions) * model_count);
	for (i=0; i < model_count; i++) {
		fscanf(fp, "%s %hd\n", models[i].pathname, &j);
		models[i].drawing = LoadModel(models[i].pathname);
		fscanf(fp, "%hhd %s\n", &models[i].has_texture, models[i].texturepath);
		if (models[i].has_texture)
			models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
		for (k=0; k < models[i].drawing.materialCount; k++)
			models[i].drawing.materials[k].shader = shader;	
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
			list_insert(&new, &asteroids, sizeof(struct model));
		else
			enemies[total_enemies++] = new;
	}
	fclose(fp);
	qsort(enemies, total_enemies, sizeof(struct model), sort_by_dist);
}

void unload_map()
{
	int i;

	total_enemies = 0;
	while (asteroids.first)
		list_remove(asteroids.first, &asteroids);
	for (i = 0; i < model_count; i++)
		while (models[i].collision_list.first)
			list_remove(models[i].collision_list.first, &models[i].collision_list);
	free(models);
	model_count = 0;
}

#define SPACESHIP_POS camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f
#define PLAY_SOUND_BY_DIST(POINT, SOUND) \
			float I = SOUND.P / Vector3Distance(POINT, (Vector3){SPACESHIP_POS}); \
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
	struct shot *ptrbullet, new_bullet;
	float now;
	int i;
	bool collision;
	Vector3 next_position;

	pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
					MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
			MatrixTranslate(SPACESHIP_POS));
	mod_spaceship.transform = pos_spaceship;
	new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
	new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
	new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
	new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
	/*player collision with asteroids*/
	collision = collision_spacecraft_asteroids();
	if (collision && !prev_collision) {
		PlaySound(collision_sound);
		prev_collision = true;
		game_state.life--;
	}
	if (!collision)
		prev_collision = false;
	/*player shots*/
	for (next = shots.first; next;) {
		ptrbullet = (struct shot *)next->data;
		// bullets moves 
		if (ptrbullet->position.z > -2 * ARRIVAL_DIST) {
			next_position = (Vector3){ptrbullet->position.x,
						 ptrbullet->position.y,
						 ptrbullet->position.z -
						 BULLET_SPEED_PER_SECOND * GetFrameTime()};
			/*bullets collisions*/
			if (collision_bullet_asteroids(ptrbullet->position, next_position, PLAYER_BULLET_SIZE)) {
				PLAY_SOUND_BY_DIST(ptrbullet->position, hit_asteroid_player_shot)
				MOVE_LIGHT_SUN(ptrbullet->light_idx)
				curr = next;
				next = next->next;
				list_remove(curr, &shots);
				continue;
			}
   			// penalty
			if (collision_bullet_field(&ptrbullet->position, &next_position, &ptre)) {
				now = GetTime();
				if (now - ((struct enemy_spacecraft*)ptre->data)->last_shooted_penalty > 0.5f) {
					for (i=0; i < MAX_CLASSES; i++)
						current_error.y[i] = 0.0f;
					current_error.y[GetRandomValue(0,3)] = 1.0f;
					for (i=0; i < INPUT_QTT; i++)
						current_error.x[i] = ((struct enemy_spacecraft*)ptre->data)->last_state[i];
					list_insert(&current_error,
						    &enemy_errors[((struct enemy_spacecraft*)ptre->data)->head],
						    sizeof(struct enemy_error));
				}
				((struct enemy_spacecraft*)ptre->data)->last_shooted_penalty = now;
				// damage
				if (collision_bullet_enemies(&ptrbullet->position, &next_position, &ptre)) {
					PLAY_SOUND_BY_DIST(ptrbullet->position, hit_enemy)
					if (--((struct enemy_spacecraft*)ptre->data)->life == 0) {
						PLAY_SOUND_BY_DIST(((struct enemy_spacecraft*)ptre->data)->shape.position, destroyed_sound)
						list_remove(ptre, &following);
					}
					MOVE_LIGHT_SUN(ptrbullet->light_idx)
					curr = next;
					next = next->next;
					list_remove(curr, &shots);
					continue;
				}
			}
			ptrbullet->position.z -= BULLET_SPEED_PER_SECOND * GetFrameTime();

			lights[ptrbullet->light_idx].position = ptrbullet->position;
			UpdateLightValues(shader, lights[ptrbullet->light_idx]);
			next = next->next;
		} else {
			MOVE_LIGHT_SUN(ptrbullet->light_idx)
			curr = next;
			next = next->next;
			list_remove(curr, &shots);
		}
	}
	now = GetTime();
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
	    now - prev_time > 1.0f / FIRING_RATE_PER_SECOND) {
		new_bullet.position.x = camera.target.x - 1.58f;
		new_bullet.position.y = camera.target.y - 1.926f;
		new_bullet.position.z = camera.target.z - 0.62 - .4f / 2;
		new_bullet.light_idx = find_light();
		lights[new_bullet.light_idx].position = new_bullet.position;
		lights[new_bullet.light_idx].color = ORANGE;
		list_insert(&new_bullet, &shots, sizeof(struct shot));
		new_bullet.position.x += 3.22f;
		new_bullet.position.y -= 0.05f;
		new_bullet.position.z -= 0.03f;
		new_bullet.light_idx = find_light();
		lights[new_bullet.light_idx].position = new_bullet.position;
		lights[new_bullet.light_idx].color = ORANGE;
		list_insert(&new_bullet, &shots, sizeof(struct shot));
		prev_time = GetTime();
#ifdef PLAY
		game_state.score -= SCORE_PER_SHOT;
		PlaySound(shot_sound);
#endif
	}
}

collision_spacecraft_asteroids()
{
	struct node *next, *next_box;
	struct list *collisions_list;
	struct model *ptrm, *current;
	Vector3 temp;
	float new_scale;

	for (next = asteroids.first; next; next = next->next) {
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

/*https://math.stackexchange.com/questions/1939423/calculate-if-vector-intersects-sphere*/
check_collision_sphere_line(p1, p2, C, R, r)
Vector3 *p1, *p2, *C;
float R, r;
{
	Vector3 U, Q;
	float a, b, c, d;
	
	// true radius 
	R += r;
	U = Vector3Normalize(Vector3Subtract(*p2, *p1));
	if ((U.z > 0.0f && p2->z > C->z - R && p1->z <= C->z + R) ||
	    (U.z <= 0.0f && p2->z < C->z + R && p1->z >= C->z - R)) {
		Q = Vector3Subtract(*p1, *C);
		a = Vector3DotProduct(U, U);
		b = 2 * Vector3DotProduct(U, Q);
		c = Vector3DotProduct(Q, Q) - R*R; 
		d = b*b - 4 * a * c;
		return d > 0.0f;
	}
	return 0;
}

collision_bullet_asteroids(bullet_prev, bullet_after, size)
Vector3 bullet_prev, bullet_after;
float size;
{
	struct node *next, *next_box;
	struct list *collisions_list;
	struct model *ptrm, *current;
	Vector3 temp;
	float new_scale;

	for (next = asteroids.first; next; next = next->next) {
		current = (struct model*)next->data;
		collisions_list = &models[current->model].collision_list;
		for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
			ptrm = (struct model*)next_box->data;
			new_scale = ptrm->scale * current->scale;
			temp = TRANFORM_SPHERE(current, ptrm->position);
			if (check_collision_sphere_line(&bullet_prev, &bullet_after, &temp, new_scale, size))
				return 1;
		}
	}
	return 0;
}

/* Replace with spheres on the player as well */
collision_bullet_player(bullet)
Vector3 *bullet;
{

	if (CheckCollisionBoxSphere(new_spaceship1, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	if (CheckCollisionBoxSphere(new_spaceship2, *bullet, ENEMY_BULLET_SIZE))
		return 1;
	return 0;
}

collision_bullet_field(bullet_prev, bullet_after, enemy)
Vector3 *bullet_prev, *bullet_after;
struct node **enemy;
{
	struct node *next;
	struct model *current;

	for (next = following.first; next; next = next->next) {
		current = &((struct enemy_spacecraft*)next->data)->shape; 
		if (check_collision_sphere_line(bullet_prev, bullet_after, &current->position,
					        current->scale * ENEMY_FIELD, PLAYER_BULLET_SIZE)) {
			*enemy = next;
			return 1;
		}
	}
	return 0;
}

collision_bullet_enemies(bullet_prev, bullet_after, enemy)
Vector3 *bullet_prev, *bullet_after;
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
			if (check_collision_sphere_line(bullet_prev, bullet_after, &temp, new_scale, PLAYER_BULLET_SIZE)) {
				*enemy = next;
				return 1;
			}
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
	bool moved, collision_field;
	int i;
	struct enemy_shot new_shot, *ptrbullet;
	Vector3 next_position;
	
	for (next_enemy = following.first; next_enemy;) {
		moved = false;
		ptrenemy = (struct enemy_spacecraft*)next_enemy->data;
		collision_field = collision_enemy_field_asteroids(&ptrenemy->shape.position, ptrenemy->shape.scale * ENEMY_FIELD);
		now = GetTime();
		if (collision_field) {
			// penalty 
			if (now - ptrenemy->last_asteroid_penalty > 1.0f) {
				ptrenemy->penalties.collision = 1;
				ptrenemy->has_penalty = 1;
				ptrenemy->last_asteroid_penalty = now;
			}
			// enemy asteroid collision
			if (collision_enemy_asteroids(ptrenemy) && now - ptrenemy->last_asteroid > 0.5f) {
				PLAY_SOUND_BY_DIST(ptrenemy->shape.position, hit_asteroid_enemy_spacecraft)
				if (--ptrenemy->life == 0) {
					PLAY_SOUND_BY_DIST(ptrenemy->shape.position, destroyed_sound)
					curr = next_enemy;
					next_enemy = next_enemy->next;
					list_remove(curr, &following);
					continue;
				}
				ptrenemy->last_asteroid = now;
			}
		}
		// enemy away from player or in the corners
		if ((Vector3Distance(ptrenemy->shape.position, (Vector3){SPACESHIP_POS}) > 
		     ptrenemy->dist + LIMIT_DISTANCE_TO_PLAYER ||
		    fabs(ptrenemy->shape.position.x) > CORNER ||
		    fabs(ptrenemy->shape.position.y > CORNER)
		    ) && now - ptrenemy->last_far > 2.0f) {
			ptrenemy->penalties.faraway = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->last_far = now;
		}
		next_enemy = next_enemy->next;
	}
	for (next = enemy_bullets.first; next;) {
		ptrbullet = (struct enemy_shot*)next->data;
		// bullet move
		if (ptrbullet->properties.position.z < MAX_DIST) {
			next_position = (Vector3){ptrbullet->properties.position.x,
						 ptrbullet->properties.position.y,
						 ptrbullet->properties.position.z +
						 ENEMY_BULLET_SPEED_PER_SECOND * GetFrameTime()};
			// hit a asteroid
			if (collision_bullet_asteroids(ptrbullet->properties.position, next_position, ENEMY_BULLET_SIZE)) {
				PLAY_SOUND_BY_DIST(ptrbullet->properties.position, hit_asteroid_enemy_shot)
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[GetRandomValue(0,3)] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ptrbullet->state[i];
				list_insert(&current_error,
					    &enemy_errors[ptrbullet->head],
					    sizeof(struct enemy_error));
				MOVE_LIGHT_SUN(ptrbullet->properties.light_idx)
				curr = next;
				next = next->next;
				list_remove(curr, &enemy_bullets);
				continue;
			}
			// hit the player
			if (collision_bullet_player(&ptrbullet->properties.position)) {
				PlaySound(hit_player);
#ifdef PLAY
				--game_state.life;
#endif
				for (i=0; i < MAX_CLASSES; i++)
					current_error.y[i] = 0.0f;
				current_error.y[4] = 1.0f;
				for (i=0; i < INPUT_QTT; i++)
					current_error.x[i] = ptrbullet->state[i];
				list_insert(&current_error, &enemy_errors[ptrbullet->head], sizeof(struct enemy_error));
				MOVE_LIGHT_SUN(ptrbullet->properties.light_idx)
				curr = next;
				next = next->next;
				list_remove(curr, &enemy_bullets);
				continue;
			}
			ptrbullet->properties.position.z += ENEMY_BULLET_SPEED_PER_SECOND * GetFrameTime();

			lights[ptrbullet->properties.light_idx].position = ptrbullet->properties.position;
			UpdateLightValues(shader, lights[ptrbullet->properties.light_idx]);
			next = next->next;
		} else {
			// bullet crossed the map
			for (i=0; i < MAX_CLASSES; i++)
				current_error.y[i] = 0.0f;
			current_error.y[GetRandomValue(0,3)] = 1.0f;
			for (i=0; i < INPUT_QTT; i++)
				current_error.x[i] = ptrbullet->state[i];
			list_insert(&current_error, &enemy_errors[ptrbullet->head], sizeof(struct enemy_error));
			MOVE_LIGHT_SUN(ptrbullet->properties.light_idx)
			curr = next;
			next = next->next;
			list_remove(curr, &enemy_bullets);
		}
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
			list_insert(&current_error, &enemy_errors[ptrenemy->head], sizeof(struct enemy_error));
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
		for (curr = asteroids.first; curr; curr = curr->next) {
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
					       (Vector3){SPACESHIP_POS}) / DIAGONAL_MAP;
		curr_state[10] = DIAGONAL_MAP;
		for (curr = shots.first; curr; curr = curr->next) {
			aux = Vector3Distance(ptrenemy->shape.position, *(Vector3*)curr->data);
			if (curr_state[10] > aux)
				curr_state[10] = aux;
		}
		curr_state[10] /= DIAGONAL_MAP;
		run(three_heads[ptrenemy->head], curr_state);
		for (i=0; i < INPUT_QTT; i++)
			ptrenemy->last_state[i] = curr_state[i];
		ptrenemy->has_penalty = 0;
		
		if (isnan(three_heads[ptrenemy->head]->network_output[0])) {
#ifdef PLAY
			current_screen = menu;
			return;
#else
			exit(0);
#endif
		}

		if (three_heads[ptrenemy->head]->network_output[0] > .3f) {
			ptrenemy->shape.position.x += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x >  MAX_DIST-10.0f)
				ptrenemy->shape.position.x =  MAX_DIST-10.0f;
			else
				moved = true;
		}
		if (three_heads[ptrenemy->head]->network_output[1] > .3f) {
			ptrenemy->shape.position.x -= VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x < -MAX_DIST+10.0f)
				ptrenemy->shape.position.x = -MAX_DIST+10.0f;
			else
				moved = true;
		}
		if (three_heads[ptrenemy->head]->network_output[2] > .3f) {
			ptrenemy->shape.position.y += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.y >  MAX_DIST-10.0f)
				ptrenemy->shape.position.y =  MAX_DIST-10.0f;
			else
				moved = true;
		}
		if (three_heads[ptrenemy->head]->network_output[3] > .3f) {
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
		if (three_heads[ptrenemy->head]->network_output[4] > .3f) {
			if (now - ptrenemy->last_shoot > 1.0f / FIRING_RATE_PER_SECOND)	{
				ptrenemy->last_shoot = now;
				new_shot.properties.position = ptrenemy->shape.position;
				new_shot.properties.position.z += 2.0f;
				new_shot.properties.light_idx = find_light();
				lights[new_shot.properties.light_idx].color = RED;
				lights[new_shot.properties.light_idx].position = new_shot.properties.position;
				for (i=0; i < INPUT_QTT; i++)
					new_shot.state[i] = ptrenemy->last_state[i];
				new_shot.head = ptrenemy->head;
				list_insert(&new_shot, &enemy_bullets, sizeof(struct enemy_shot));
				PLAY_SOUND_BY_DIST(new_shot.properties.position, enemy_shot_sound);
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
	struct node *next;
	struct model *draw_model;
	Vector3 position, new_bullet, aux_bullet;
	float inc, limit, dist;
	int i;

	rlDisableBackfaceCulling();
	rlDisableDepthMask();
		DrawModel(mod_skybox, (Vector3){0}, 1.0f, WHITE);
	rlEnableBackfaceCulling();
	rlEnableDepthMask();

	for (next = asteroids.first; next; next = next->next) {
		draw_model = (struct model*)next->data;
		DrawModelRotate(models[draw_model->model].drawing,
				draw_model->position,
				draw_model->angles,
				draw_model->scale,
				WHITE);
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
			new_enemy.head = GetRandomValue(0, 2);
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
				MODEL_COLOR);
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
			  (Vector3){SPACESHIP_POS}, 
			  Vector3Distance(draw_model->position, 
				          (Vector3){SPACESHIP_POS}) >
			                  ((struct enemy_spacecraft*)next->data)->dist + LIMIT_DISTANCE_TO_PLAYER? RED : BLUE);
#endif
	}
	DrawModel(mod_spaceship, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, MODEL_COLOR);
	for (next = shots.first; next; next = next->next) {
		aux_bullet = new_bullet = *(Vector3*)next->data;
		new_bullet.z += .2f;
		aux_bullet.z -= .4f;
		DrawCapsule(new_bullet, aux_bullet, .125f, 4, 4, ORANGE);
	}
	for (next = enemy_bullets.first; next; next = next->next)
		DrawSphere(*(Vector3*)next->data, ENEMY_BULLET_SIZE, RED);
}

collision_enemy_asteroids(enemy)
struct enemy_spacecraft *enemy;
{
	struct node *next, *next_box, *next_enemy_box;
	struct list *collisions_list;
	struct model *ptrm, *current, *eptrm, *ecurrent;
	Vector3 temp, etemp;
	float new_scale, enew_scale;
	
	ecurrent = &enemy->shape;
	for (next = asteroids.first; next; next = next->next) {
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

collision_enemy_field_asteroids(enemy_pos, enemy_range)
Vector3 *enemy_pos;
float enemy_range;
{
	struct node *next, *next_box;
	struct list *collisions_list;
	struct model *ptrm, *current;
	Vector3 temp;
	float new_scale;

	for (next = asteroids.first; next; next = next->next) {
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

find_light()
{
	int i;

	for (i=1; i < MAX_LIGHTS; i++)
		if (lights[i].position.x == SUN_X)
			return i;
	return -1;
}

/*******************************************************************************************
*
*   raylib [models] example - Skybox loading and drawing
*
*   Example originally created with raylib 1.8, last time updated with raylib 4.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
void load_skybox()
{
	mod_skybox = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	mod_skybox.materials[0].shader = LoadShader("shaders/skybox.vs", "shaders/skybox.fs");

	SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
	SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "doGamma"), (int[1]) { 0 }, SHADER_UNIFORM_INT);
	SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "vflipped"), (int[1]) { 0 }, SHADER_UNIFORM_INT);
	
	Shader shader_cubemap = LoadShader("shaders/cubemap.vs", "shaders/cubemap.fs");

	SetShaderValue(shader_cubemap, GetShaderLocation(shader_cubemap, "equirectangularMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

	Image img = LoadImage("models/cubemap.png");
	mod_skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);    // CUBEMAP_LAYOUT_PANORAMA
	UnloadImage(img);
}
