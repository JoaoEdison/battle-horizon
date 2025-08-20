/*
  Battle Horizon is a 3D space battle game in Raylib
  Copyright (C) 2023-2025  João E. R. Manica
  
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

#define PLAY

#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include "defs.h"
#include "../lib/ocrc/src/neural_net.h"
#include "../lib/ocrc/src/model.h"

#ifdef PLAY
#include "ui.c"
#else
#include "translate.c"
#endif

#include "battle_horizon.c"
#define RLIGHTS_IMPLEMENTATION
#include "../res/shaders/rlights.h"
#include "camera.c"

struct enemy_spacecraft {
    model shape;
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

#define WEIGHTS_LOCATION "res/weights"

#ifndef PLAY
#undef HEAD_COUNT
#define HEAD_COUNT 1
#endif

neural_net_bignet_ptr three_heads[HEAD_COUNT];

list_linkedlist enemy_errors[HEAD_COUNT] = { 0 };
struct enemy_error {
    float x[INPUT_QTT], y[MAX_CLASSES];
} current_error;

models_with_collisions *models;
short model_count;
list_linkedlist asteroids = { 0 };
list_linkedlist following = { 0 };
list_linkedlist shots = { 0 };
list_linkedlist enemy_bullets = { 0 };
struct enemy_spacecraft new_enemy;

int screen_width, screen_height;
Camera camera;
Model mod_spaceship, mod_skybox;
BoundingBox new_spaceship1, new_spaceship2;
void (*current_screen)();
bool exit_game = 0;

struct sound_I {
    Sound sound;
    float P;
} destroyed_sound, enemy_shot_sound,
  hit_asteroid_player_shot, hit_asteroid_enemy_shot,
  hit_enemy, hit_asteroid_enemy_spacecraft;

Sound shot_sound, victory_sound, defeat_sound, hit_player, collision_sound;

Shader shader_cubemap, shader_light;
Light lights[MAX_LIGHTS];

#define MODEL_COLOR WHITE

#define SUN_X   380.0f
#define SUN_Y   320.0f
#define SUN_Z (700.0f)

main(argc, argv)
char *argv[];
{
    void load_map(), init_network(), menu(), game(), load_skybox();
    int i;
#ifdef PLAY
    void load_scores(), save_scores();

    game_state.life = MAX_LIFE;
    new_enemy.life = ENEMY_LIFE;
#else
    new_enemy.life = 20;
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
    InitAudioDevice();

    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();
#ifdef PLAY
    config_layout(screen_width, screen_height);
    ui_sound = LoadSound("res/sounds/ui.wav");
    SetSoundVolume(ui_sound, 2.0f);
#endif
    shot_sound = LoadSound("res/sounds/laser3.mp3");
    SetSoundVolume(shot_sound, 0.08f);
    collision_sound = LoadSound("res/sounds/hit-asteroid-spacecraft.wav");
    SetSoundVolume(collision_sound, 0.2f);
    victory_sound = LoadSound("res/sounds/win.mp3");
    defeat_sound = LoadSound("res/sounds/lost.mp3");
    SetSoundVolume(defeat_sound, 0.5f);
    hit_player = LoadSound("res/sounds/hit-player.wav");
    SetSoundVolume(hit_player, 0.1f);
    
    hit_enemy.sound = LoadSound("res/sounds/hit-enemy.mp3");
    hit_enemy.P = 120.0f;
    hit_asteroid_player_shot.sound = LoadSound("res/sounds/hit-asteroid-player-shot.mp3");
    hit_asteroid_player_shot.P = 150.0f;
    hit_asteroid_enemy_shot.sound = LoadSound("res/sounds/hit-asteroid-enemy-shot.mp3");
    hit_asteroid_enemy_shot.P = 75.0f;
    hit_asteroid_enemy_spacecraft.sound = LoadSound("res/sounds/hit-asteroid-enemy-spacecraft.wav");
    hit_asteroid_enemy_spacecraft.P = 12.0f;
    enemy_shot_sound.sound = LoadSound("res/sounds/enemy-shot.mp3");
    enemy_shot_sound.P = 12.0f;
    destroyed_sound.sound = LoadSound("res/sounds/enemy-destroyed.mp3");
    destroyed_sound.P = 12.0f;
    
    mod_spaceship = LoadModel("res/models/spacecraft.glb");
    //shaders
    load_skybox();    
    shader_light = LoadShader("res/shaders/lighting.vs", "res/shaders/lighting.fs");
    shader_light.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader_light, "viewPos");
    for (i=0; i < mod_spaceship.materialCount; i++)
        mod_spaceship.materials[i].shader = shader_light;
    // create directional light facing 0,0,0    
    lights[0] = CreateLight(LIGHT_DIRECTIONAL, (Vector3){SUN_X, SUN_Y, SUN_Z}, Vector3Zero(), WHITE, shader_light);
    lights[0].enabled = 1;
    UpdateLightValues(shader_light, lights[0]);
    // create all possible lights in sun
    for (i=1; i < MAX_LIGHTS; i++)
        lights[i] = CreateLight(LIGHT_POINT, Vector3Zero(), Vector3Zero(), WHITE, shader_light);
    SetTargetFPS(60);
#ifdef PLAY
    load_scores();
    ToggleFullscreen();
    current_screen = menu;
#else
    load_map("res/maps/easy.map");
    current_screen = game;
    DisableCursor();
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
#ifdef PLAY
    UnloadSound(ui_sound);
    save_scores();
#else
    if (following.size)
        save_weights(three_heads[((struct enemy_spacecraft*)following.first->data)->head], WEIGHTS_LOCATION);
#endif
    CloseAudioDevice();
}

model enemies[10];
int total_enemies = 0, first_enemy = 0;
sort_by_dist(x, y)
void *x, *y;
{
    return ((model*)y)->position.z - ((model*)x)->position.z;
}

#ifdef PLAY
void load_scores()
{
    score_entry *new_score;
    int minutes, seconds, easy;
    char c, *ptrname;
    FILE *fp;
    
    array_allocate(scores_easy, sizeof(score_entry), 10);
    array_allocate(scores_hard, sizeof(score_entry), 10);
    if ((fp = fopen("res/scores.csv", "rb"))) {
        for (;;) {
            if (fscanf(fp, "%d", &easy) == EOF)
                break;
            if (easy) {
                new_score = (score_entry*) ARRAY_LAST_SPACE(scores_easy);
                scores_easy.nmemb++;
            } else {
                new_score = (score_entry*) ARRAY_LAST_SPACE(scores_hard);
                scores_hard.nmemb++;
            }
            fscanf(fp, ",%d,", &new_score->score);
            for (ptrname = new_score->name; (c = fgetc(fp)) != ','; *ptrname++ = c);
            *ptrname = '\0';
            fscanf(fp, "%d/%d/%d,%d:%d\n", &new_score->day, &new_score->month, &new_score->year, &minutes, &seconds);
            new_score->seconds = seconds;
            new_score->seconds += minutes * 60;
        }
        fclose(fp);
    }
}

void save_scores()
{
    score_entry *new_score;
    FILE *fp;
    int i;

    if ((fp = fopen("res/scores.csv", "w"))) {
        for (i=0; i < scores_easy.nmemb; i++) {
            new_score = (score_entry*) ARRAY_AT(scores_easy, i);
            fprintf(fp, "%d,%d,%s,%d/%d/%d,%d:%d\n", 1, 
                                 new_score->score,
                                 new_score->name,
                                 new_score->day,
                                 new_score->month,
                                 new_score->year,
                                 new_score->seconds / 60,
                                 new_score->seconds % 60);
        }
        for (i=0; i < scores_hard.nmemb; i++) {
            new_score = (score_entry*) ARRAY_AT(scores_hard, i);
            fprintf(fp, "%d,%d,%s,%d/%d/%d,%d:%d\n", 0, 
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
    void game(), load_map(), unload_map(), replay();

    BeginDrawing();
        ClearBackground(BLACK);
        if (screen != 5)
            DrawTextEx(font, t_t_game, (Vector2){
                    screen_width/2 - MeasureTextEx(font, t_t_game, font.baseSize * title_size, spacing).x/2,
                    MeasureTextEx(font, t_t_game, font.baseSize * title_size, spacing).y/4
                    }, font.baseSize * title_size, spacing, BLUE);
        switch (screen) {
            case 1:
                screen = draw_play();
                break;
            case 2:
                load_map(easy_map? "res/maps/easy.map" : "res/maps/hard.map");
                replay();
                current_screen = game;
                break;
            case 3:
                screen = draw_about();
                break;
            case 4:
                exit_game = 1;
                break;
            case 5:
                screen = score_table();
                break;
            case 6:
                unload_map();
                screen = draw_play();
                break;
            case 7:
                replay();
                current_screen = game;
                break;
            default:
                screen = draw_menu();
                break;
        }
    EndDrawing();    
}

void replay()
{
    int i;

    SetMousePosition(screen_width / 2, screen_height / 2);
    DisableCursor();

    camera.position = (Vector3){ 0.0f, 5.0f, 0.0f };
    camera.target = (Vector3){0.0f, 4.5f,-1.0f };
    camera.up = (Vector3){ .0f, 6.0f, .0f };
    game_state.life = MAX_LIFE;
    game_state.state = 0;
    game_state.prev_time = 0.0f;
    game_state.total_shots = game_state.total_enemies_destroyed = 0;
    while (following.first)
        remove_linkedlist(following.first, &following);
    while (enemy_bullets.first)
        remove_linkedlist(enemy_bullets.first, &enemy_bullets);
    while (shots.first)
        remove_linkedlist(shots.first, &shots);
    for (i=1; i < MAX_LIGHTS; i++) {
        lights[i].enabled = 0;
        UpdateLightValues(shader_light, lights[i]);
    }
    first_enemy = 0;
    game_state.time = GetTime();
    ZERO_VEL
}
#endif

void game()
{
    void manage_player(), manage_enemies(), draw_scene();
    node_linkedlist *next;
    struct enemy_error *ptrerr;
    float now, prev_back;
    int i;

    prev_back = 0.0f;
    UpdateMyCamera(&camera, GetFrameTime());
    
    manage_player();    
    manage_enemies();
    SetShaderValue(shader_light, shader_light.locs[SHADER_LOC_VECTOR_VIEW], (float[3]){camera.position.x, camera.position.y, camera.position.z}, SHADER_UNIFORM_VEC3);
#ifdef PLAY
    if (!game_state.state) {
        if (GetTime() - game_state.time >= DEADLINE_SECS-1.0f)
            game_state.state = -1;
        if (-camera.target.z > ARRIVAL_DIST)
            game_state.state = following.size? -1 : 1;
        else if (!game_state.life)
            game_state.state = -1;
        if (game_state.state) {
            PlaySound(game_state.state == 1? victory_sound : defeat_sound);
            game_state.time = (int)(DEADLINE_SECS + game_state.time - GetTime());
            game_state.score = game_state.life * SCORE_PER_LIFE +
                       game_state.total_enemies_destroyed * SCORE_PER_ENEMY +
                       game_state.time * SCORE_PER_SECOND +
                       game_state.total_shots * SCORE_PER_SHOT;
            game_state.score += total_enemies * ENEMY_LIFE * -SCORE_PER_SHOT;
        }
    }
    game_state.distance = -camera.target.z;
#endif
    now = GetTime();
    if (now - prev_back > 5.0f) {
        for (i=0; i < HEAD_COUNT; i++) {
            three_heads[i]->N = enemy_errors[i].size;
            neural_net_clear_backpr(three_heads[i]);
            for (next = enemy_errors[i].first; next; next = next->next) {
                ptrerr = (struct enemy_error*)next->data;
                neural_net_backpr(three_heads[i], ptrerr->x, ptrerr->y);
            }
            while (enemy_errors[i].first)
                remove_linkedlist(enemy_errors[i].first, &enemy_errors[i]);
            neural_net_apply_backpr(three_heads[i]);
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
            screen = 5;
        }
#endif
    EndDrawing();
}

unsigned layers1[] = {INPUT_QTT, 10, MAX_CLASSES};
neural_net_create_network_arr nets = {{layers1, sizeof(layers1)/sizeof(unsigned), INPUT_QTT, 1, -1}};

void init_network()
{
    int i;

    if (FileExists(WEIGHTS_LOCATION))
        for (i=0; i < HEAD_COUNT; i++) {
            three_heads[i] = neural_net_load_weights(WEIGHTS_LOCATION, 1);
            neural_net_ini_backpr(three_heads[i], 2);
        }
    else
        for (i=0; i < HEAD_COUNT; i++) {
            three_heads[i] = neural_net_init_net_topology(nets, 1, 1);
            neural_net_init_random_weights(three_heads[i]);
            neural_net_ini_backpr(three_heads[i], 2);
        }
}

void load_map(name)
char name[];
{
    model new;
    short i, j, k, enemy_idx;
    FILE *fp;
    
    enemy_idx = -1;
    if (!(fp = fopen(name, "r"))) {
        fprintf(stderr, "[%s:%d] could not open file (%s).\n", __func__, __LINE__, name);
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%hd\n", &model_count);
    models = malloc(sizeof(models_with_collisions) * model_count);
    for (i=0; i < model_count; i++) {
        fscanf(fp, "%s %hd\n", models[i].pathname, &j);
        models[i].drawing = LoadModel(models[i].pathname);
        fscanf(fp, "%d %s\n", &models[i].has_texture, models[i].texturepath);
        if (models[i].has_texture)
            models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
        for (k=0; k < models[i].drawing.materialCount; k++)
            models[i].drawing.materials[k].shader = shader_light;    
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
            insert_linkedlist(&new, &models[i].collision_list, sizeof(model));
        }
    }
    fscanf(fp, "%hd\n", &i);
    while (i--) {
        fscanf(fp, "%d\n", &new.model_idx);
        fscanf(fp, "%f %f %f %f\n",
                &new.position.x,
                &new.position.y,
                &new.position.z,
                &new.scale);
        fscanf(fp, "%f %f %f\n", &new.angles.x,
                &new.angles.y, &new.angles.z);
        if (enemy_idx != new.model_idx)
            insert_linkedlist(&new, &asteroids, sizeof(model));
        else
            enemies[total_enemies++] = new;
    }
    fclose(fp);
    qsort(enemies, total_enemies, sizeof(model), sort_by_dist);
}

void unload_map()
{
    int i;

    total_enemies = 0;
    while (asteroids.first)
        remove_linkedlist(asteroids.first, &asteroids);
    for (i = 0; i < model_count; i++)
        while (models[i].collision_list.first)
            remove_linkedlist(models[i].collision_list.first, &models[i].collision_list);
    free(models);
    model_count = 0;
}

#define SPACESHIP_POS camera.target.x, camera.target.y - 1.8f, camera.target.z + 0.5f

#define PLAY_SOUND_BY_DIST(POINT, SOUND) \
            float intensity = SOUND.P / Vector3Distance(POINT, (Vector3){SPACESHIP_POS}); \
            SetSoundVolume(SOUND.sound, intensity); \
            PlaySound(SOUND.sound);

#define DISABLE_LIGHT(LIGHT) \
    lights[LIGHT].enabled = 0; \
    UpdateLightValues(shader_light, lights[LIGHT]); \
    LIGHT = 0;

void manage_player()
{
    static Matrix pos_spaceship;
        /* The minimum point is the front, left and bottom.
     * There is a 90-degree rotation, counterclockwise and on the y-axis*/
    static BoundingBox box_spaceship1 = {
        { .46f,-0.2f,-2.0f},
        {-.9f,    0.1f, 2.0f}
    };
    static BoundingBox box_spaceship2 = {
        { 0.8f,0.1f,-0.5f},
        {-0.7f,0.8f, 0.5f}
    };
    static bool prev_collision = 0;
    static float prev_time = 0;
    node_linkedlist *curr, *next, *ptre;
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
        prev_collision = 1;
        ZERO_VEL
#ifdef PLAY
        game_state.life--;
#endif
    }
    if (!collision)
        prev_collision = 0;
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
            if (collision_bullet_asteroids(&ptrbullet->position, &next_position, PLAYER_BULLET_SIZE)) {
                PLAY_SOUND_BY_DIST(ptrbullet->position, hit_asteroid_player_shot)
                if (ptrbullet->light_idx > 0) {
                    DISABLE_LIGHT(ptrbullet->light_idx)
                }
                curr = next;
                next = next->next;
                remove_linkedlist(curr, &shots);
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
                    insert_linkedlist(&current_error,
                            &enemy_errors[((struct enemy_spacecraft*)ptre->data)->head],
                            sizeof(struct enemy_error));
                }
                ((struct enemy_spacecraft*)ptre->data)->last_shooted_penalty = now;
                // damage
                if (collision_bullet_enemies(&ptrbullet->position, &next_position, &ptre)) {
                    PLAY_SOUND_BY_DIST(ptrbullet->position, hit_enemy)
                    if (--((struct enemy_spacecraft*)ptre->data)->life == 0) {
                        PLAY_SOUND_BY_DIST(((struct enemy_spacecraft*)ptre->data)->shape.position, destroyed_sound)
                        remove_linkedlist(ptre, &following);
#ifdef PLAY
                        game_state.total_enemies_destroyed++;
#endif
                    }
                    if (ptrbullet->light_idx > 0) {
                        DISABLE_LIGHT(ptrbullet->light_idx)
                    }
                    curr = next;
                    next = next->next;
                    remove_linkedlist(curr, &shots);
                    continue;
                }
            }
            ptrbullet->position = next_position;
            if (ptrbullet->light_idx > 0) {
                if (ptrbullet->position.z > camera.position.z - 2*DISTANCE_FROM_PLAYER) {
                    lights[ptrbullet->light_idx].position = ptrbullet->position;
                    UpdateLightValues(shader_light, lights[ptrbullet->light_idx]);
                } else if (lights[ptrbullet->light_idx].enabled) {
                    DISABLE_LIGHT(ptrbullet->light_idx)
                }
            }
            next = next->next;
        } else {
            curr = next;
            next = next->next;
            remove_linkedlist(curr, &shots);
        }
    }
    now = GetTime();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        now - prev_time > 1.0f / FIRING_RATE_PER_SECOND) {
        new_bullet.position.x = camera.target.x - 1.58f;
        new_bullet.position.y = camera.target.y - 1.926f;
        new_bullet.position.z = camera.target.z - 0.62 - .4f / 2;
        new_bullet.light_idx = find_light();
        lights[new_bullet.light_idx].color = PLAYER_BULLET_COLOR;
        lights[new_bullet.light_idx].enabled = 1;
        insert_linkedlist(&new_bullet, &shots, sizeof(struct shot));
        new_bullet.position.x += 3.22f;
        new_bullet.position.y -= 0.05f;
        new_bullet.position.z -= 0.03f;
        new_bullet.light_idx = find_light();
        lights[new_bullet.light_idx].color = PLAYER_BULLET_COLOR;
        lights[new_bullet.light_idx].enabled = 1;
        insert_linkedlist(&new_bullet, &shots, sizeof(struct shot));
        prev_time = GetTime();
        PlaySound(shot_sound);
#ifdef PLAY
        game_state.total_shots += 2;
#endif
    }
}

collision_spacecraft_asteroids()
{
    node_linkedlist *next, *next_box;
    list_linkedlist *collisions_list;
    model *ptrm, *current;
    Vector3 temp;
    float new_scale;

    for (next = asteroids.first; next; next = next->next) {
        current = (model*)next->data;
        collisions_list = &models[current->model_idx].collision_list;
        for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
            ptrm = (model*)next_box->data;
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

/* https://math.stackexchange.com/questions/1939423/calculate-if-vector-intersects-sphere */
check_collision_sphere_line(p1, p2, C, R, r)
Vector3 *p1, *p2, *C;
float R, r;
{
    Vector3 U, Q;
    float a, b, c, d;
    
    /* true radius */
    R += r;
    /* see if the sphere is between the bullet's trajectory
     * somehow, it seems that it doesn't check in some direction... */
    if (CheckCollisionBoxSphere((BoundingBox){*p2, *p1}, *C, R) ||
        CheckCollisionBoxSphere((BoundingBox){*p1, *p2}, *C, R)) {
        U = Vector3Normalize(Vector3Subtract(*p2, *p1));
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
Vector3 *bullet_prev, *bullet_after;
float size;
{
    node_linkedlist *next, *next_box;
    model *ptrm, *current;
    Vector3 temp;

    for (next = asteroids.first; next; next = next->next) {
        current = (model*)next->data;
        for (next_box = models[current->model_idx].collision_list.first; next_box; next_box = next_box->next) {
            ptrm = (model*)next_box->data;
            temp = TRANFORM_SPHERE(current, ptrm->position);
            if (check_collision_sphere_line(bullet_prev, bullet_after, &temp, ptrm->scale * current->scale, size))
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
node_linkedlist **enemy;
{
    node_linkedlist *next;
    model *current;

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
node_linkedlist **enemy;
{
    node_linkedlist *next, *next_box;
    list_linkedlist *collisions_list;
    model *ptrm, *current;
    Vector3 temp;
    float new_scale;

    for (next = following.first; next; next = next->next) {
        current = &((struct enemy_spacecraft*)next->data)->shape; 
        collisions_list = &models[current->model_idx].collision_list;
        for (next_box = collisions_list->first; next_box; next_box = next_box->next) {
            ptrm = (model*)next_box->data;
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

void update_state(enemy)
struct enemy_spacecraft *enemy;
{
    node_linkedlist *next;
    float aux;
    float *curr_state;
    
    curr_state = enemy->last_state;
    curr_state[0] = enemy->shape.position.x / MAX_DIST;
    curr_state[1] = enemy->shape.position.y / MAX_DIST;
    curr_state[2] = (ARRIVAL_DIST / 2 + enemy->shape.position.z) / (-ARRIVAL_DIST / 2);
    curr_state[3] = Vector3Distance(enemy->shape.position,
                       (Vector3){SPACESHIP_POS}) / DIAGONAL_MAP;
    curr_state[4] = curr_state[5] = curr_state[6] = DIAGONAL_MAP;
    for (next = asteroids.first; next; next = next->next) {
        aux = Vector3Distance(enemy->shape.position,
                      ((model*)next->data)->position);
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
    curr_state[7] = camera.target.x / MAX_DIST;
    curr_state[8] = (camera.target.y - 1.8f) / MAX_DIST;
    curr_state[9] = (ARRIVAL_DIST / 2 + camera.target.z + 0.5f) / (-ARRIVAL_DIST / 2);
    curr_state[10] = DIAGONAL_MAP;
    for (next = shots.first; next; next = next->next) {
        aux = Vector3Distance(enemy->shape.position, *(Vector3*)next->data);
        if (curr_state[10] > aux)
            curr_state[10] = aux;
    }
    curr_state[10] /= DIAGONAL_MAP;
}

void manage_enemies()
{
    node_linkedlist *next, *curr, *next_enemy;
    struct enemy_spacecraft *ptrenemy;
    float now;
    bool moved, collision_field;
    int i;
    struct enemy_shot new_shot, *ptrbullet;
    model *asteroid;
    Vector3 next_position;
    
    for (next_enemy = following.first; next_enemy;) {
        moved = 0;
        ptrenemy = (struct enemy_spacecraft*)next_enemy->data;
        collision_field = collision_enemy_field_asteroids(&ptrenemy->shape.position, ptrenemy->shape.scale * ENEMY_FIELD, &asteroid);
        now = GetTime();
        if (collision_field) {
            // penalty 
            if (now - ptrenemy->last_asteroid_penalty > 1.0f) {
                ptrenemy->penalties.collision = 1;
                ptrenemy->has_penalty = 1;
                ptrenemy->last_asteroid_penalty = now;
            }
            // enemy asteroid collision
            if (now - ptrenemy->last_asteroid > 0.5f)
                if (collision_enemy_asteroid(ptrenemy, asteroid)) {
                    PLAY_SOUND_BY_DIST(ptrenemy->shape.position, hit_asteroid_enemy_spacecraft)
                    if (--ptrenemy->life == 0) {
                        PLAY_SOUND_BY_DIST(ptrenemy->shape.position, destroyed_sound)
                        curr = next_enemy;
                        next_enemy = next_enemy->next;
                        remove_linkedlist(curr, &following);
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
            if (collision_bullet_asteroids(&ptrbullet->properties.position, &next_position, ENEMY_BULLET_SIZE)) {
                PLAY_SOUND_BY_DIST(ptrbullet->properties.position, hit_asteroid_enemy_shot)
                for (i=0; i < MAX_CLASSES; i++)
                    current_error.y[i] = 0.0f;
                current_error.y[GetRandomValue(0,3)] = 1.0f;
                for (i=0; i < INPUT_QTT; i++)
                    current_error.x[i] = ptrbullet->state[i];
                insert_linkedlist(&current_error,
                        &enemy_errors[ptrbullet->head],
                        sizeof(struct enemy_error));
                if (ptrbullet->properties.light_idx > 0) {
                    DISABLE_LIGHT(ptrbullet->properties.light_idx)
                }
                curr = next;
                next = next->next;
                remove_linkedlist(curr, &enemy_bullets);
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
                insert_linkedlist(&current_error, &enemy_errors[ptrbullet->head], sizeof(struct enemy_error));
                if (ptrbullet->properties.light_idx > 0) {
                    DISABLE_LIGHT(ptrbullet->properties.light_idx)
                }
                curr = next;
                next = next->next;
                remove_linkedlist(curr, &enemy_bullets);
                continue;
            }
            ptrbullet->properties.position = next_position;
            
            if (ptrbullet->properties.light_idx > 0) {
                if (ptrbullet->properties.position.z < camera.position.z + 2*DISTANCE_FROM_PLAYER) {
                    lights[ptrbullet->properties.light_idx].position = ptrbullet->properties.position;
                    UpdateLightValues(shader_light, lights[ptrbullet->properties.light_idx]);
                } else if (lights[ptrbullet->properties.light_idx].enabled) {
                    DISABLE_LIGHT(ptrbullet->properties.light_idx)
                }
            }
            next = next->next;
        } else {
            // bullet crossed the map
            for (i=0; i < MAX_CLASSES; i++)
                current_error.y[i] = 0.0f;
            current_error.y[GetRandomValue(0,3)] = 1.0f;
            for (i=0; i < INPUT_QTT; i++)
                current_error.x[i] = ptrbullet->state[i];
            insert_linkedlist(&current_error, &enemy_errors[ptrbullet->head], sizeof(struct enemy_error));
            curr = next;
            next = next->next;
            remove_linkedlist(curr, &enemy_bullets);
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
            insert_linkedlist(&current_error, &enemy_errors[ptrenemy->head], sizeof(struct enemy_error));
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
        update_state(ptrenemy);
        neural_net_run(three_heads[ptrenemy->head], ptrenemy->last_state);
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
                moved = 1;
        }
        if (three_heads[ptrenemy->head]->network_output[1] > .3f) {
            ptrenemy->shape.position.x -= VELOCITY_ENEMY * GetFrameTime();
            if (ptrenemy->shape.position.x < -MAX_DIST+10.0f)
                ptrenemy->shape.position.x = -MAX_DIST+10.0f;
            else
                moved = 1;
        }
        if (three_heads[ptrenemy->head]->network_output[2] > .3f) {
            ptrenemy->shape.position.y += VELOCITY_ENEMY * GetFrameTime();
            if (ptrenemy->shape.position.y >  MAX_DIST-10.0f)
                ptrenemy->shape.position.y =  MAX_DIST-10.0f;
            else
                moved = 1;
        }
        if (three_heads[ptrenemy->head]->network_output[3] > .3f) {
            ptrenemy->shape.position.y -= VELOCITY_ENEMY * GetFrameTime();
            if (ptrenemy->shape.position.y <  -MAX_DIST+10.0f)
                ptrenemy->shape.position.y =  -MAX_DIST+10.0f;
            else
                moved = 1;
        }
        /*shot*/
        now = GetTime();
        if (now - ptrenemy->last_move > 2.0f && !moved) {
            ptrenemy->penalties.dont_move = 1;
            ptrenemy->has_penalty = 1;
            ptrenemy->last_move = now;
        }
        if (three_heads[ptrenemy->head]->network_output[4] > .3f) {
            if (now - ptrenemy->last_shoot > 1.0f / FIRING_RATE_PER_SECOND)    {
                ptrenemy->last_shoot = now;
                new_shot.properties.position = ptrenemy->shape.position;
                new_shot.properties.position.z += 2.0f;
                new_shot.properties.light_idx = find_light();
                lights[new_shot.properties.light_idx].color = ENEMY_BULLET_COLOR;
                lights[new_shot.properties.light_idx].enabled = 1;
                for (i=0; i < INPUT_QTT; i++)
                    new_shot.state[i] = ptrenemy->last_state[i];
                new_shot.head = ptrenemy->head;
                insert_linkedlist(&new_shot, &enemy_bullets, sizeof(struct enemy_shot));
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
    node_linkedlist *next;
    model *draw_model;
    Vector3 position, new_bullet, aux_bullet;
    float inc, limit, dist;
    int i;

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
        DrawModel(mod_skybox, (Vector3){0}, 1.0f, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();

    for (next = asteroids.first; next; next = next->next) {
        draw_model = (model*)next->data;
        DrawModelRotate(models[draw_model->model_idx].drawing,
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
            new_enemy.head = GetRandomValue(0, HEAD_COUNT-1);
            insert_linkedlist(&new_enemy, &following, sizeof(struct enemy_spacecraft));
            first_enemy++;
            continue;
        }
        /*enemy spawn -- arrival*/
        inc = INITIAL_ENEMY_DIST * fabs(draw_model->position.z + limit - camera.target.z) /
                              (-draw_model->position.z);
        position = draw_model->position;
        position.x = position.x > .0f? position.x + inc : position.x - inc;
        position.y = position.y > .0f? position.y + inc : position.y - inc;
        DrawModelRotate(models[draw_model->model_idx].drawing,
                position,
                draw_model->angles,
                draw_model->scale,
                MODEL_COLOR);
    }
    for (next = following.first; next; next = next->next) {
        draw_model = &((struct enemy_spacecraft*)next->data)->shape;
        draw_model->position.z -= ((struct enemy_spacecraft*)next->data)->dist - fabs(camera.target.z - draw_model->position.z);
        DrawModelRotate(models[draw_model->model_idx].drawing,
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
        DrawCapsule(new_bullet, aux_bullet, .125f, 4, 4, PLAYER_BULLET_COLOR);
    }
    for (next = enemy_bullets.first; next; next = next->next)
        DrawSphere(*(Vector3*)next->data, ENEMY_BULLET_SIZE, ENEMY_BULLET_COLOR);
}

collision_enemy_asteroid(enemy, asteroid)
struct enemy_spacecraft *enemy;
model *asteroid;
{
    node_linkedlist *next, *next_enemy_box;
    model *ptrm, *eptrm, *ecurrent;
    Vector3 temp, etemp;
    float new_scale, enew_scale;
    
    ecurrent = &enemy->shape;
    for (next = models[asteroid->model_idx].collision_list.first; next; next = next->next) {
        ptrm = (model*)next->data;
        new_scale = ptrm->scale * asteroid->scale;
        temp = TRANFORM_SPHERE(asteroid, ptrm->position);
        for (next_enemy_box = models[enemy->shape.model_idx].collision_list.first; next_enemy_box; next_enemy_box = next_enemy_box->next) {
            eptrm = (model*)next->data;
            enew_scale = eptrm->scale * enemy->shape.scale;
            etemp = TRANFORM_SPHERE(ecurrent, eptrm->position);
            if (CheckCollisionSpheres(temp, new_scale, etemp, enew_scale))
                return 1;
        }
    }
    return 0;
}

collision_enemy_field_asteroids(enemy_pos, enemy_range, asteroid)
Vector3 *enemy_pos;
float enemy_range;
model **asteroid;
{
    node_linkedlist *next, *next_box;
    model *ptrm, *current;
    Vector3 temp;

    for (next = asteroids.first; next; next = next->next) {
        current = (model*)next->data;
        for (next_box = models[current->model_idx].collision_list.first; next_box; next_box = next_box->next) {
            ptrm = (model*)next_box->data;
            temp = TRANFORM_SPHERE(current, ptrm->position);
            if (CheckCollisionSpheres(temp, ptrm->scale * current->scale, *enemy_pos, enemy_range)) {
                *asteroid = current;
                return 1;
            }
        }
    }
    return 0;
}

int max = 0;

find_light()
{
    int i;

    for (i=1; i < MAX_LIGHTS; i++)
        if (!lights[i].enabled) {
            if (i > max) {
                max = i;
                printf("%d\n", i);
            }
            return i;
        }
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
    mod_skybox.materials[0].shader = LoadShader("res/shaders/skybox.vs", "res/shaders/skybox.fs");

    SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
    SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "doGamma"), (int[1]) { 0 }, SHADER_UNIFORM_INT);
    SetShaderValue(mod_skybox.materials[0].shader, GetShaderLocation(mod_skybox.materials[0].shader, "vflipped"), (int[1]) { 0 }, SHADER_UNIFORM_INT);
    
    Shader shader_cubemap = LoadShader("res/shaders/cubemap.vs", "res/shaders/cubemap.fs");

    SetShaderValue(shader_cubemap, GetShaderLocation(shader_cubemap, "equirectangularMap"), (int[1]){ 0 }, SHADER_UNIFORM_INT);

    Image img = LoadImage("res/models/cubemap.png");
    mod_skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);    // CUBEMAP_LAYOUT_PANORAMA
    UnloadImage(img);
}
