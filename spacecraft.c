#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

/* TODO:
 * danos;
 * tiros;
 * naves inimigas;
 * movimentos;
 * menu;
 * texturas;
 * sons.
 * */

main()
{
  void UpdateMyCamera();
  bool collision = false;
  int screen_width, screen_height;
  Camera camera;
  Matrix pos_spaceship;
  
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

  Model mod_rock = LoadModel("modelos/rock1.glb");
  Vector3 rock_trans = {0.0f, 5.0f, -50.0f};
  Matrix rock_pos = MatrixMultiply(MatrixScale(10.0f, 10.0f, 10.0f),
		  MatrixTranslate(rock_trans.x, rock_trans.y, rock_trans.z));
  mod_rock.transform = rock_pos;
  
  Model mod_spaceship = LoadModel("modelos/spacecraft.glb");
  // o mínimo é a frente, esquerda e baixo
  // há uma rotação de 90 graus, no sentido anti-horário e no eixo y
  BoundingBox box_spaceship1 = {
    { .46f,-0.2f,-2.0f},
    {-.9f,  0.1f, 2.0f}
  };
  BoundingBox new_spaceship1, new_spaceship2;
  BoundingBox box_spaceship2 = {
    { 0.8f,0.1f,-0.5f},
    {-0.7f,0.8f, 0.5f}
  };
  
  while (!WindowShouldClose()) {
    UpdateMyCamera(&camera, CAMERA_THIRD_PERSON);
    //printf("%.2f %.2f %.2f\n", camera.target.x, camera.target.y,
    //		camera.target.z);
    pos_spaceship = MatrixMultiply(MatrixMultiply(MatrixScale(1.5f, 1.5f, 1.5f),
			      MatrixRotate((Vector3){0.0f, 1.0f, 0.0f}, 1.57f)),
	   		MatrixTranslate(camera.target.x, camera.target.y - 1.8f,
	  			camera.target.z + 0.5f));
    mod_spaceship.transform = pos_spaceship;
    new_spaceship1.min = Vector3Transform(box_spaceship1.min, pos_spaceship);
    new_spaceship1.max = Vector3Transform(box_spaceship1.max, pos_spaceship);
    new_spaceship2.min = Vector3Transform(box_spaceship2.min, pos_spaceship);
    new_spaceship2.max = Vector3Transform(box_spaceship2.max, pos_spaceship);
    
    #define RADIUS 9.8f 
    collision = CheckCollisionBoxSphere(new_spaceship1, rock_trans, RADIUS+1.0f);
    collision = CheckCollisionBoxSphere(new_spaceship2, rock_trans, RADIUS+1.0f);

    BeginDrawing();
      ClearBackground(RAYWHITE);
      BeginMode3D(camera);
	DrawSphere((Vector3){camera.position.x+250.0f, camera.position.y+150.0f,
			camera.position.z-700.0f}, 100.0f, YELLOW);
	DrawModel(mod_spaceship, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, RED);
	DrawModel(mod_rock, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, GRAY);
        DrawGrid(1300, 1.0f);
	DrawBoundingBox(new_spaceship1, PURPLE);
	DrawBoundingBox(new_spaceship2, BLACK);
	if (collision) {
	  DrawSphere(new_spaceship1.min, 0.1f, RED);
	  DrawSphere(new_spaceship1.max, 0.1f, ORANGE);
	} else {
	  DrawSphere(new_spaceship1.min, 0.1f, BLUE);
	  DrawSphere(new_spaceship1.max, 0.1f, GREEN);
	}
        DrawSphereWires(rock_trans, RADIUS, 20, 20, LIME);
      EndMode3D();
      DrawFPS(10, 10);
    EndDrawing();
  }
  CloseWindow();
}
