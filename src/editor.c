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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#include "../include/defs.h"

#include "battle_horizon.c"
#include "camera.c"

#define MAX_MODELS 10

models_with_collisions models[MAX_MODELS];
short model_count;
int enemy_model;

#define NEW_MODEL new_model.position = camera.target; \
                  new_model.angles = (Vector3){ .0f, .0f, .0f }; \
                  new_model.scale = 10.0f;

void load_models()
{
    model new_box = { 0 };
    int i;
    
    i = 0;
    strcpy(models[i].pathname, "res/models/asteroid1.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    strcpy(models[i].texturepath, "res/models/black-white-details-moon-texture.png");
    models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
    models[i].has_texture = 1;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 1.0f;
    new_box.position = (Vector3){ 0 };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    
    i++;
    strcpy(models[i].pathname, "res/models/asteroid2.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    strcpy(models[i].texturepath, "res/models/black-white-details-moon-texture.png");
    models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
    models[i].has_texture = 1;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 1.0f;
    new_box.position = (Vector3){ 0 };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    
    i++;
    strcpy(models[i].pathname, "res/models/asteroid3.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    strcpy(models[i].texturepath, "res/models/black-stone-texture.png");
    models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
    models[i].has_texture = 1;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = .8f;
    new_box.position = (Vector3){ -.2f, .4f, .2f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = .9f;
    new_box.position = (Vector3){ .2f, -.2f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));

    
    i++;
    strcpy(models[i].pathname, "res/models/asteroid4.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    strcpy(models[i].texturepath, "res/models/black-stone-texture.png");
    models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
    models[i].has_texture = 1;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 0.7f;
    new_box.position = (Vector3){ 0.0f, 0.0f, 0.2f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 0.6f;
    new_box.position = (Vector3){ 0.0f,-0.5f, 0.0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    
    i++;
    strcpy(models[i].pathname, "res/models/asteroid5.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    strcpy(models[i].texturepath, "res/models/black-stone-texture.png");
    models[i].drawing.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(models[i].texturepath);
    models[i].has_texture = 1;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 0.7f;
    new_box.position = (Vector3){ 0 };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 0.6f;
    new_box.position = (Vector3){ 0.0f,  0.5f, 0.2f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    
    i++;
    strcpy(models[i].pathname, "res/models/asteroid6.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    models[i].has_texture = 0;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 1.5f;
    new_box.position = (Vector3){ 0 };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 1.5f;
    new_box.position = (Vector3){ 0.0f, 1.0f, 0.0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 1.5f;
    new_box.position = (Vector3){ 0.0f, -1.0f, 0.0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    
    i++;
    enemy_model = i;
    strcpy(models[i].pathname, "res/models/enemy.glb");
    models[i].drawing = LoadModel(models[i].pathname);
    models[i].has_texture = 0;
    models[i].collision_list.head = NULL;
    models[i].collision_list.nmemb = 0;
    new_box.scale = 1.5f;
    new_box.position = (Vector3){ 0 };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 1.1f;
    new_box.position = (Vector3){ 2.3f, .0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = 1.1f;
    new_box.position = (Vector3){-2.3f, .0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = .8f;
    new_box.position = (Vector3){ 2.1f, 2.0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = .8f;
    new_box.position = (Vector3){-2.1f, 2.0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = .8f;
    new_box.position = (Vector3){ 2.2f,-2.0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));
    new_box.scale = .8f;
    new_box.position = (Vector3){-2.2f,-2.0f, .0f };
    linkedlist_appendlloc(&models[i].collision_list, &new_box, sizeof(model));

    model_count = 7;
}

linkedlist_list drawing = { 0 };
Camera camera = { 0 };

typedef struct {
    Vector3 position;
    float radius;
    linkedlist_list group_selection;
} group;

group new_group = { 0 };

linkedlist_list groups = { 0 };
linkedlist_list selection = { 0 };
linkedlist_node *selected;
linkedlist_list ln = { 0 };
Vector3 lastpos;

bool save_flag = false, open_flag = false, move_flag = false;
char name[MAX_MAP_NAME_LEN];

model new_model;

main()
{
    void map_edit(), add_objects(), draw_scene(), get_in_volume();
    void trans_single(), trans_selection(), trans_group();
    linkedlist_node *get_selected();
    linkedlist_node *next, *curr;
    
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
            UpdateCameraEditor(&camera, GetFrameTime());
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
            if (selection.nmemb)
                trans_selection();
            if (groups.nmemb)
                trans_group();
            if (IsKeyDown(KEY_LEFT_CONTROL)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (selected) {
                        LINKEDLIST_MOVE_ONE(&selection, &drawing, selected);
                        selected = NULL;
                    }
                    if ((next = get_selected(&drawing)))
                        LINKEDLIST_MOVE_ONE(&selection, &drawing, next);
                } else if (IsKeyPressed(KEY_A)) {
                    for (next=drawing.head; next;) {
                        if (((model*)next->value)->model_idx != enemy_model) {
                            linkedlist_append(&selection, next->value);
                            next = linkedlist_delete(&drawing, next);
                            continue;
                        }
                        next = next->next;
                    }
                } else if (IsKeyPressed(KEY_S))
                    save_flag = true;
                else if (IsKeyPressed(KEY_O))
                    open_flag = true;
            } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                selected = get_selected(&drawing);
                if (!selected)
                    linkedlist_move_all_last_first(&drawing, &selection);
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
        new_model.model_idx = number_pressed;
        linkedlist_appendlloc(&drawing, &new_model, sizeof(model));
    }
    if (IsKeyPressed(KEY_G)) {
        new_group.position = camera.target;
        linkedlist_appendlloc(&groups, &new_group, sizeof(group));
    }
}

void map_edit()
{
    void save_map(), open_map();
    static char real_name[MAX_MAP_NAME_LEN+14];
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
            strcpy(real_name, "res/maps/");
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

linkedlist_node *get_selected(l)
linkedlist_list *l;
{
    linkedlist_list *collisions;
    linkedlist_node *next, *next_collision;
    model *target;
    model *ptrm;
    float new_scale;
    Vector3 temp;

    for (next = l->head; next; next = next->next) {
        target = (model*)next->value;
        collisions = &models[target->model_idx].collision_list;
        for (next_collision = collisions->head; next_collision; next_collision = next_collision->next) {
            ptrm = (model*)next_collision->value;
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
    linkedlist_node *next1, *next2;
    model *current;
    FILE *fp;
    short i, count = 0, map[MAX_MODELS];
    linkedlist_list present = { 0 };
    
    for (i = 0; i < MAX_MODELS; i++)
        map[i] = -1;
    for (next1 = drawing.head; next1; next1 = next1->next) {
        i = ((model*)next1->value)->model_idx;
        if (map[i] < 0) {
            map[i] = count++;
            linkedlist_appendlloc(&present, &i, sizeof(short));
        }
    }
    fp = fopen(name, "wb");
    fprintf(fp, "%hd\n", count);
    for (next1 = present.head; next1; next1 = next1->next) {
        i = *(short*)next1->value;
        fprintf(fp, "%s %hd\n", models[i].pathname, models[i].collision_list.nmemb);
        fprintf(fp, "%hhd %s\n", models[i].has_texture, models[i].has_texture? models[i].texturepath : "0");
        for (next2 = models[i].collision_list.head; next2; next2 = next2->next) {
            current = (model*)next2->value;
            fprintf(fp, "%.4f %.4f %.4f %.4f\n",
                    current->position.x,
                    current->position.y,
                    current->position.z,
                    current->scale);
        }
    }
    fprintf(fp, "%hd\n", drawing.nmemb);
    for (next1 = drawing.head; next1; next1 = next1->next) {
        current = (model*)next1->value;
        fprintf(fp, "%hhu\n", map[current->model_idx]);
        fprintf(fp, "%.4f %.4f %.4f %.4f\n",
                current->position.x,
                current->position.y,
                current->position.z,
                current->scale);
        fprintf(fp, "%.4f %.4f %.4f\n", current->angles.x,
                current->angles.y, current->angles.z);
    }
    fclose(fp);
    linkedlist_erasefree(&present);
}

void open_map(name)
char name[];
{
    model new;
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
        fscanf(fp, "%u\n", &new.model_idx);
        new.model_idx = map[new.model_idx];
        fscanf(fp, "%f %f %f %f\n",
                &new.position.x,
                &new.position.y,
                &new.position.z,
                &new.scale);
        fscanf(fp, "%f %f %f\n", &new.angles.x,
                &new.angles.y, &new.angles.z);
        linkedlist_appendlloc(&drawing, &new, sizeof(model));
    }
    fclose(fp);
}

#define DRAW_MODEL(PTR, COLOR) \
        draw_model = (model*)PTR->value; \
        DrawModelRotate(models[draw_model->model_idx].drawing, draw_model->position, draw_model->angles, draw_model->scale, COLOR); \
        draw_collisions_wires(draw_model, &models[draw_model->model_idx].collision_list);

void draw_scene()
{
    model *draw_model;
    linkedlist_node *next, *current;

    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
            for (next = drawing.head; next; next = next->next) {
                if (next == selected) {
                    DRAW_MODEL(next, PURPLE)
                } else {
                    DRAW_MODEL(next, GRAY)    
                }
            }
            for (next = selection.head; next; next = next->next) {
                DRAW_MODEL(next, RED)
            }
            for (next = groups.head; next; next = next->next) {
                DrawSphereWires(((group*)next->value)->position, ((group*)next->value)->radius, 24, 24, BLACK);
                current = ((group*)next->value)->group_selection.head;
                for (; current; current = current->next) {
                    DRAW_MODEL(current, ORANGE)
                }
            }
            DrawSphereWires(camera.target, .3f, 12, 12, ORANGE);
            DrawSphere((Vector3){.0f, .0f, .0f}, 5.0f, BLUE);
            DrawCubeWires((Vector3){0.0f, 0.0f, -ARRIVAL_DIST / 2}, MAX_DIST*2, MAX_DIST*2, ARRIVAL_DIST, RED);
        EndMode3D();
        DrawText(TextFormat("Total de Modelos: %d", drawing.nmemb + selection.nmemb), 10, 30, 20, BLACK);
        DrawText("[Botao Esquerdo] selecionar\n"
             "[Botao Direito] mover\n"
             "[1 - 6] adicionar modelo de asteroide\n"
             "[7] adicionar nave inimiga\n"
             "[CTRL  + c] copiar\n"
             "[CTRL  + v] colar\n"
             "[CTRL  + a] selecionar todos os asteroides\n"
             "[CTRL  + s] salvar mapa\n"
             "[CTRL  + o] abrir mapa\n"
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
        ((model*)selected->value)->position = camera.target;
    if (IsKeyDown(KEY_X))
        ((model*)selected->value)->angles.x += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyDown(KEY_Y))
        ((model*)selected->value)->angles.y += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyDown(KEY_Z))
        ((model*)selected->value)->angles.z += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (IsKeyDown(KEY_EQUAL))
            ((model*)selected->value)->scale += 1.0f;
        else if (IsKeyDown(KEY_MINUS))
            ((model*)selected->value)->scale -= 1.0f;
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_C)) {
            new_model.model_idx = ((model*)selected->value)->model_idx;
            new_model.angles = ((model*)selected->value)->angles;
            new_model.scale = ((model*)selected->value)->scale;
        } else if (IsKeyPressed(KEY_V)) {
            new_model.position = camera.target;
            linkedlist_appendlloc(&drawing, &new_model, sizeof(model));
        }
    }
    if (IsKeyPressed(KEY_DELETE)) {
        LINKEDLIST_DELETEFREE(&drawing, selected);
        selected = NULL;
    }
}

#define MOVE_MANY(GROUP, PTR) \
        for (next = GROUP.head; next; next = next->next) { \
            ((PTR*)next->value)->position.x += camera.target.x - lastpos.x; \
            ((PTR*)next->value)->position.y += camera.target.y - lastpos.y; \
            ((PTR*)next->value)->position.z += camera.target.z - lastpos.z; \
        }

void trans_selection()
{
    linkedlist_node *next;

    if (move_flag) {
        MOVE_MANY(selection, model)
        lastpos = camera.target;
    }
    if (IsKeyDown(KEY_X))
        for (next = selection.head; next; next = next->next)
            ((model*)next->value)->angles.x += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyDown(KEY_Y))
        for (next = selection.head; next; next = next->next)
            ((model*)next->value)->angles.y += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyDown(KEY_Z))
        for (next = selection.head; next; next = next->next)
            ((model*)next->value)->angles.z += IsKeyDown(KEY_MINUS)? -.8f : .8f;
    if (IsKeyPressed(KEY_R))
        for (next = selection.head; next; next = next->next) {
            ((model*)next->value)->angles.x = GetRandomValue(0, 359);
            ((model*)next->value)->angles.y = GetRandomValue(0, 359);
            ((model*)next->value)->angles.z = GetRandomValue(0, 359);
        }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (IsKeyDown(KEY_EQUAL))
            for (next = selection.head; next; next = next->next)
                ((model*)next->value)->scale += 1.0f;
        else if (IsKeyDown(KEY_MINUS))
            for (next = selection.head; next; next = next->next)
                ((model*)next->value)->scale -= 1.0f;
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_C)) {
            for (next = selection.head; next; next = next->next) {
                new_model.model_idx = ((model*)next->value)->model_idx;
                new_model.angles = ((model*)next->value)->angles;
                new_model.scale = ((model*)next->value)->scale;
                new_model.position = ((model*)next->value)->position;
                linkedlist_appendlloc(&ln, &new_model, sizeof(model));
            }
            lastpos = camera.target;
        } else if (IsKeyPressed(KEY_V)) {
            for (next = ln.head; next; next = next->next) {
                ((model*)next->value)->position.x += camera.target.x - lastpos.x;
                ((model*)next->value)->position.y += camera.target.y - lastpos.y;
                ((model*)next->value)->position.z += camera.target.z - lastpos.z;
                linkedlist_append(&drawing, next->value);
            }
            linkedlist_erase(&ln);
        }
    }
    if (IsKeyPressed(KEY_DELETE))
        linkedlist_erasefree(&selection);
}

void get_in_volume(in_list, out_list, group_shape)
linkedlist_list *in_list, *out_list;
group *group_shape;
{
    linkedlist_node *next, *next_collision;
    model *target, *ptrm;
    linkedlist_list *collisions;
    Vector3 temp;
    float new_scale;
    bool found;

    for (next = in_list->head; next;) {
        found = false;
        target = (model*)next->value;
        collisions = &models[target->model_idx].collision_list;
        for (next_collision = collisions->head; next_collision; next_collision = next_collision->next) {
            ptrm = (model*)next_collision->value;
            temp = TRANFORM_SPHERE(target, ptrm->position)
            new_scale = ptrm->scale * target->scale;
            if (CheckCollisionSpheres(temp, new_scale, group_shape->position, group_shape->radius)) {
                linkedlist_append(out_list, next->value);
                next = linkedlist_delete(in_list, next);
                found = true;
                break;
            }
        }
        if (!found)
            next = next->next;
    }
}

#define RELEASE_GROUP_SELECTION(LIST) \
    linkedlist_move_all_last_first(&drawing, LIST)

#define ROTATE_GROUP(AXIS, DEGREE) \
    for (next = groups.head; next; next = next->next) { \
        current = ((group*)next->value)->group_selection.head; \
        for (; current; current = current->next) { \
            direction = Vector3RotateByAxisAngle( \
                Vector3Subtract(((model*)current->value)->position, \
                        ((group*)next->value)->position), \
                AXIS, \
                DEGREE); \
            ((model*)current->value)->position = Vector3Add(direction, ((group*)next->value)->position); \
        } \
    }

void trans_group()
{
    linkedlist_node *next, *current;
    linkedlist_list *l;
    Vector3 direction;

    if (IsKeyPressed(KEY_B))
        for (next = groups.head; next; next = next->next)
            get_in_volume(&drawing, &(((group*)next->value)->group_selection), next->value);
    if (IsKeyPressed(KEY_V))
        for (next = groups.head; next; next = next->next) {
            l = &((group*)next->value)->group_selection;
            linkedlist_move_all_last_first(&selection, l);
        }
    if (IsKeyPressed(KEY_R))
        for (next = groups.head; next; next = next->next)
            RELEASE_GROUP_SELECTION(&((group*)next->value)->group_selection);
    if (IsKeyPressed(KEY_F))
        while (groups.head) {
            RELEASE_GROUP_SELECTION(&((group*)groups.head->value)->group_selection);
            LINKEDLIST_DELETEFREE(&groups, groups.head);
        }
    if (move_flag) {
        MOVE_MANY(groups, group)
        for (current = groups.head; current; current = current->next)
            MOVE_MANY(((group*)current->value)->group_selection, model)
        lastpos = camera.target;
    }
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        if (IsKeyDown(KEY_EQUAL)) {
            for (next = groups.head; next; next = next->next) {
                ((group*)next->value)->radius += 1.0f;
                current = ((group*)next->value)->group_selection.head;
                for (; current; current = current->next) {
                    direction = Vector3Normalize(Vector3Subtract(((model*)current->value)->position, ((group*)next->value)->position));
                    ((model*)current->value)->position.x += direction.x;
                    ((model*)current->value)->position.y += direction.y;
                    ((model*)current->value)->position.z += direction.z;
                }
            }
        } else if (IsKeyDown(KEY_MINUS))
            for (next = groups.head; next; next = next->next) {
                ((group*)next->value)->radius -= 1.0f;
                current = ((group*)next->value)->group_selection.head;
                for (; current; current = current->next) {
                    direction = Vector3Normalize(Vector3Subtract(((group*)next->value)->position,
                                                     ((model*)current->value)->position));
                    ((model*)current->value)->position.x += direction.x;
                    ((model*)current->value)->position.y += direction.y;
                    ((model*)current->value)->position.z += direction.z;
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
