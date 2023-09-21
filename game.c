#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "spacecraft.c"
#include "defs.h"
#include <float.h>
#include "ia/neural_img.h"

/* TODO:
 * talvez mudar a função de ativação da última camada;
 * barra de vida;
 * menu;
 * texturas;
 * sons.
 * */

#define BULLET_SPEED_PER_SECOND 200.0f
#define ENEMY_BULLET_SPEED_PER_SECOND 1.0f
#define FIRING_RATE_PER_SECOND 3.0f 

struct models_with_collisions *models;
short model_count;
struct list rocks = { 0 };
struct list enemies = { 0 };
struct list following = { 0 };
struct list shots = { 0 };
BoundingBox new_spaceship1, new_spaceship2;
int life = 5;

Camera camera;

#define INITIAL_DIST 10000.0f
#define LIMINAL (100.0f + 70.0f * following.size)

main(argc, argv)
char *argv[];
{
	void load_map(), init_network(), manage_enemies();
	bool collision = false, prev_collision = false;
	int screen_width, screen_height;
	Matrix pos_spaceship;
	struct node *next, *curr;
	struct model *draw_model;
	Vector3 new_bullet, *ptrbul, aux_bullet, aux_vec3;
	double prev_time, now;
	float inc;
	struct enemy_spacecraft new_enemy;
	struct node *ptre;
	int i;

	SetRandomSeed(3);
	init_network();

	prev_time = GetTime();
	new_enemy.life = 120;
	new_enemy.last_shoot = .0f;
	new_enemy.bullets.size = 0;
	new_enemy.color = GRAY;
	new_enemy.bullets.last = new_enemy.bullets.first = NULL;
	new_enemy.ammunition = 10;
	new_enemy.penalties.collision = 0;
	new_enemy.penalties.shooted = 0;
	new_enemy.penalties.error = 0;
	new_enemy.penalties.dont_shoot = 0;
	for (i=0; i < INPUT_QTT; i++)
		new_enemy.last_state[i] = 0;
	new_enemy.has_penalty = 0;

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

	load_map(argv[1]);
	
	while (!WindowShouldClose()) {
		UpdateMyCamera(&camera, GetFrameTime());
		pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
						MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
		 		MatrixTranslate(camera.target.x, camera.target.y - 1.8f,
					camera.target.z + 0.5f));
		mod_spaceship.transform = pos_spaceship;
		new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
		new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
		new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
		new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
		// colisão do jogador com as rochas
		collision = collision_spacecraft_rocks();
		if (collision && !prev_collision) {
			prev_collision = true;
			life--;
		}
		if (!collision)
			prev_collision = false;
		//tiros do jogador
		for (next = shots.first; next;) {
			ptrbul = (Vector3*)next->data;
			// colisão das balas
			if (collision_bullet_rocks(ptrbul)) {
				curr = next;
				next = next->next;
				list_remove(curr, &shots);
				continue;
			}
			if (collision_bullet_enemies(ptrbul, &ptre)) {
				((struct enemy_spacecraft*)ptre->data)->penalties.shooted = true;
				((struct enemy_spacecraft*)ptre->data)->has_penalty = true;
				if (--((struct enemy_spacecraft*)ptre->data)->life == 0)
					list_remove(ptre, &following);
				curr = next;
				next = next->next;
				list_remove(curr, &shots);
				continue;
			}
			// movimento das balas
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

		manage_enemies();

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
					aux_vec3 = draw_model->position;
					aux_vec3.x = aux_vec3.x > .0f? aux_vec3.x + inc : aux_vec3.x - inc;
					aux_vec3.y = aux_vec3.y > .0f? aux_vec3.y + inc : aux_vec3.y - inc;
					DrawModelRotate(models[draw_model->model].drawing,
							aux_vec3,
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
							((struct enemy_spacecraft*)next->data)->color);
					draw_collisions_wires(draw_model, &models[draw_model->model].collision_list);
					DrawSphereWires((Vector3){draw_model->position.x,
							draw_model->position.y,
							draw_model->position.z},
							draw_model->scale * 5,
							4, 4,
							BLACK);
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
						DrawSphere(*(Vector3*)next->data, 4.0f, RED);
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
	save_weights();
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
			temp = TRANFORM_SPHERE(current);
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
			temp = TRANFORM_SPHERE(current);
			if (CheckCollisionSpheres(*bullet, 1.0f, temp, new_scale))
				return 1;
		}
	}
	return 0;
}

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
			temp = TRANFORM_SPHERE(current);
			if (CheckCollisionSpheres(*bullet, 1.0f, temp, new_scale)) {
				*enemy = next;
				return 1;
			}
		}
	}
	return 0;
}

collision_bullet_player(bullet)
Vector3 *bullet;
{

	if (CheckCollisionBoxSphere(new_spaceship1, *bullet, 4.0f))
		return 1;
	if (CheckCollisionBoxSphere(new_spaceship2, *bullet, 4.0f))
		return 1;
	return 0;
}

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
			temp = TRANFORM_SPHERE(current);
			if (CheckCollisionSpheres(temp, new_scale, *enemy_pos, enemy_range))
				return 1;
		}
	}
	return 0;
}

/*última camada deve ter o mesmo que a saída???*/
unsigned layers1[] = {7, 7, 5};
struct create_network nets[] = {{layers1, sizeof(layers1)/sizeof(unsigned), INPUT_QTT, 1, -1}};
void init_network()
{
	init_net_topology(nets, 1, 1);
	if (FileExists("weights"))
		load_weights(1);
	else
		init_random_weights();
	ini_backpr(1);
	clear_backpr();
}

#define VELOCITY_ENEMY 70.0f

struct enemy_shot {
	Vector3 position;
	float state[INPUT_QTT];
};

/*posição do jogador e posição da bala mais próxima*/
/*mudar a função de ativação da última camada*/

void manage_enemies()
{
	struct node *next, *curr, *next_enemy;
	struct enemy_spacecraft *ptrenemy;
	float aux, curr_state[INPUT_QTT], now, expected[MAX_CLASSES];
	int i;
	unsigned char did_backpr;
	struct enemy_shot new_shot, *ptrbullet;
	
	for (next_enemy = following.first; next_enemy; next_enemy = next_enemy->next) {
		ptrenemy = (struct enemy_spacecraft*)next_enemy->data;
		
		//verificar se o inimigo colidiu com alguma rocha
		if (collision_enemy_rocks(&ptrenemy->shape.position, ptrenemy->shape.scale * 5)) {
			ptrenemy->penalties.collision = 1;
			ptrenemy->has_penalty = 1;
			ptrenemy->color = RED;
		} else
			ptrenemy->color = GRAY;

		for (next = ptrenemy->bullets.first; next;) {
			ptrbullet = (struct enemy_shot*)next->data;
			// colisão das balas
			if (collision_bullet_rocks(ptrbullet->position)) {
				ptrenemy->penalties.error = 1;
				ptrenemy->has_penalty = 1;
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			if (collision_bullet_player(ptrbullet->position)) {
				--life;
				for (i=0; i < MAX_CLASSES; i++)
					expected[i] = 0.0f;
				expected[4] = 1.0f;
				backpr(ptrbullet->state, expected);
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
				continue;
			}
			// movimento das balas
			if (ptrbullet->position.z < MAX_DIST) {
				ptrbullet->position.z += BULLET_SPEED_PER_SECOND * GetFrameTime();
				next = next->next;
				continue;
			} else {
				ptrenemy->penalties.error = 1;
				ptrenemy->has_penalty = 1;
				curr = next;
				next = next->next;
				list_remove(curr, &ptrenemy->bullets);
			}
		}
	}
	did_backpr = 0;
	for (next = following.first; next; next = next->next) {
		ptrenemy = (struct enemy_spacecraft*)next->data;
		if (ptrenemy->has_penalty) {
			for (i=0; i < MAX_CLASSES; i++)
				expected[i] = 0.0f;
			if (ptrenemy->penalties.collision ||
			    ptrenemy->penalties.shooted ||
			    ptrenemy->penalties.error) {
				expected[0] = GetRandomValue(0, 1);
				expected[1] = !expected[0];
				expected[2] = GetRandomValue(0, 1);
				expected[3] = !expected[0];
			}
			if (ptrenemy->penalties.dont_shoot)
				expected[4] = 1.0f;
			backpr(ptrenemy->last_state, expected);
			ptrenemy->penalties.collision = 0; 
			ptrenemy->penalties.shooted = 0;
			ptrenemy->penalties.error = 0;
			ptrenemy->penalties.dont_shoot = 0;
			did_backpr = 1;
		}
	}
	if (did_backpr) {
		apply_backpr();
		clear_backpr();
	}
	/*pegar a posição de três pedras próximas*/
	/*curr_state[0-2] está normalizado entre -1 e 1*/
	/*curr_state[3-6] está normalizado entre 0 e 1*/
	for (next = following.first; next; next = next->next) {
		ptrenemy = (struct enemy_spacecraft*)next->data;
		curr_state[3] = Vector3Distance(ptrenemy->shape.position,
					       (Vector3){camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f}) / DIAGONAL_MAP;
		curr_state[6] = curr_state[5] = curr_state[4] = FLT_MAX;
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
		curr_state[0] = ptrenemy->shape.position.x / MAX_DIST;
		curr_state[1] = ptrenemy->shape.position.y / MAX_DIST;
		curr_state[2] = (ARRIVAL_DIST / 2 + ptrenemy->shape.position.z) / (-ARRIVAL_DIST / 2);
		curr_state[4] /= DIAGONAL_MAP;
		curr_state[5] /= DIAGONAL_MAP;
		curr_state[6] /= DIAGONAL_MAP;
		run(curr_state);
		for (i=0; i < INPUT_QTT; i++)
			ptrenemy->last_state[i] = curr_state[i];
		ptrenemy->has_penalty = 0;
		/*log*/
		/*
		for (i=0; i < INPUT_QTT; i++)
			printf("%f ", curr_state[i]);
		putchar('\n');
		for (i=0; i < MAX_CLASSES; i++)
			printf("%f ", network_output[i]);
		putchar('\n');
		*/
		if (network_output[0] > .3f) {
			ptrenemy->shape.position.x += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x >  MAX_DIST-10.0f)
				ptrenemy->shape.position.x =  MAX_DIST-10.0f;
		}
		if (network_output[1] > .3f) {
			ptrenemy->shape.position.x -= VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.x < -MAX_DIST+10.0f)
				ptrenemy->shape.position.x = -MAX_DIST+10.0f;
		}
		if (network_output[2] > .3f) {
			ptrenemy->shape.position.y += VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.y >  MAX_DIST-10.0f)
				ptrenemy->shape.position.y =  MAX_DIST-10.0f;
		}
		if (network_output[3] > .3f) {
			ptrenemy->shape.position.y -= VELOCITY_ENEMY * GetFrameTime();
			if (ptrenemy->shape.position.y <  -MAX_DIST+10.0f)
				ptrenemy->shape.position.y =  -MAX_DIST+10.0f;
		}
		/*tiro*/
		now = GetTime();
		if (ptrenemy->ammunition) {
			if (network_output[4] > .3f) {
				if (now - ptrenemy->last_shoot > 1.0f / FIRING_RATE_PER_SECOND)	{
					ptrenemy->last_shoot = now;
					ptrenemy->ammunition--;
					new_shot.position = ptrenemy->shape.position;
					for (i=0; i < INPUT_QTT; i++)
						new_shot.state[i] = ptrenemy->last_state[i];	
					list_insert(&new_shot, &ptrenemy->bullets, sizeof(struct enemy_shot));
				}
			} else if (now - ptrenemy->last_shoot > 1.0f) {
				ptrenemy->penalties.dont_shoot = 1;
				ptrenemy->has_penalty = 1;
				ptrenemy->last_shoot = now;
			}
		}
	}
}
