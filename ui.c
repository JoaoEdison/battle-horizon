#include <raylib.h>
#include "defs.h"

#define MAX_LIFE 5
#define STEP_SIZE 100
#define PADDING 6
#define WIDTH_LIFE 20

void draw_ui(width, height, distance, life)
float distance;
{
	if (distance > ARRIVAL_DIST) {
		//carregar a fonte do exemplo e mostrar a mensagem		
	} else {
		DrawRectangle(1, height/2 - (MAX_LIFE * STEP_SIZE + 2*PADDING) / 2, WIDTH_LIFE + 2*PADDING,
				(MAX_LIFE * STEP_SIZE + 2*PADDING), GRAY);
		DrawRectangle(PADDING+1, height/2 - life * STEP_SIZE / 2, WIDTH_LIFE, life * STEP_SIZE, RED);
		
		DrawRectangle(width - WIDTH_LIFE - 2 * PADDING - 1, height/2 - (MAX_LIFE * STEP_SIZE + 2*PADDING) / 2, WIDTH_LIFE + 2*PADDING,
				(MAX_LIFE * STEP_SIZE + 2*PADDING), GRAY);
		Rectangle rec = {
			width-PADDING-1,
			height/2 + MAX_LIFE * STEP_SIZE / 2,
			WIDTH_LIFE,
			distance / ARRIVAL_DIST * STEP_SIZE * MAX_LIFE
		};
		DrawRectanglePro(rec, (Vector2){0.0f, 0.0f}, 180.0f, ORANGE);
	}
}
