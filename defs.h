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
/*map dimensions*/
#define MAX_DIST 120.0f
#define ARRIVAL_DIST (MAX_DIST*10)
#define DIAGONAL_MAP 1247.076f
#define CORNER (MAX_DIST - 20.0f)
/*player*/
#define MAX_LIFE 5
#define SPACECRAFT_SPEED 20.0f
#define SPACECRAFT_SPEED_MAX 50.0f
#define VELOCITY_SIDE 8.0f
#define VELOCITY_FORWARD VELOCITY_SIDE
#define LIMIT_VELOCITY 200.0f
#define DEADLINE_SECS (ARRIVAL_DIST / SPACECRAFT_SPEED - 4.0f)
#define PLAYER_BULLET_SIZE 1.0f
#define PLAYER_BULLET_COLOR SKYBLUE
/*shots*/
#define BULLET_SPEED_PER_SECOND 450.0f
#define FIRING_RATE_PER_SECOND 3.0f
/*enemies*/
#define HEAD_COUNT 3
#define INITIAL_ENEMY_DIST 10000.0f
#define DISTANCE_FROM_PLAYER 150.0f
#define ENEMY_BULLET_SPEED_PER_SECOND 150.0f
#define VELOCITY_ENEMY 40.0f
#define ENEMY_BULLET_SIZE 3.6f
#define ENEMY_BULLET_COLOR RED
#define ENEMY_LIFE 4
#define ENEMY_FIELD 5.0f
#define LIMIT_DISTANCE_TO_PLAYER 10.0f
/*score*/
#define SCORE_PER_SHOT -2
#define SCORE_PER_SECOND 4
#define SCORE_PER_LIFE 30
#define SCORE_PER_ENEMY 30

#define STRINGIZE(X) STRINGIZE_H(X)
#define STRINGIZE_H(X) #X
