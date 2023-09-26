#include <raylib.h>
#include "defs.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.c"

#define MAX_LIFE 5
#define STEP_SIZE 100
#define PADDING 6
#define WIDTH_LIFE 20
#define SPACING 4
#define WIN_MESSAGE_SIZE 8

draw_ui(width, height, distance, life, font)
float distance;
Font *font;
{
	static float prev_win = 0.0f;
	float now;
	
	if (distance < ARRIVAL_DIST) {
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
	} else {
		now = GetTime();
		if (prev_win == 0.0f)
			prev_win = now;
		else if (prev_win + 3.0f <= now)
			return 1;
		DrawTextEx(*font, "VOCE VENCEU", (Vector2){
				width/2 - MeasureTextEx(*font, "VOCE VENCEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				height/2 - MeasureTextEx(*font, "VOCE VENCEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font->baseSize * WIN_MESSAGE_SIZE, SPACING, GREEN);
	}
	return 0;
}

#define TITLE_SIZE 12
#define BUTTON_FONT_SIZE 5
#define BUTTON_PADDING_SIDES 20
#define BUTTON_PADDING_VERT 10

draw_menu(width, height, font)
Font *font;
{
	int ret = 0;

	Rectangle rec;
	GuiSetFont(*font);
	GuiSetStyle(DEFAULT, TEXT_SIZE, font->baseSize * BUTTON_FONT_SIZE);
	rec.width = MeasureTextEx(*font, "Jogar", font->baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.height = MeasureTextEx(*font, "Jogar", font->baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.x = (width - rec.width) / 2;
	rec.y = height / 2 - 150;
	if (GuiButton(rec, "Jogar"))
		ret = 1;
	rec.y += 100;
	if (GuiButton(rec, "Sobre"))
		ret = 2;
	rec.y += 100;
	if (GuiButton(rec, "Sair"))
		ret = 3;
	return ret;
}

#define PANEL_MARGIN_OUTSIDE 100
#define PANEL_MARGIN_INSIDE 20
#define PANEL_MARGIN_TOP 300
#define PANEL_MARGIN_BOTTON 180
#define PANEL_HEADER_SIZE 3

char input[20] = "nome";

draw_play(width, height, font)
Font *font;
{
	Vector2 mouse;
	int ret = 1;
	bool edit;

	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);
	GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	Rectangle rec, input_rec;
	rec.width = width / 2 - PANEL_MARGIN_INSIDE - PANEL_MARGIN_OUTSIDE;
	rec.height = height - PANEL_MARGIN_TOP - PANEL_MARGIN_BOTTON;
	rec.x = PANEL_MARGIN_OUTSIDE;
	rec.y = PANEL_MARGIN_TOP;
	GuiPanel(rec, "Facil");
	rec.x = width / 2 + PANEL_MARGIN_INSIDE;
	GuiPanel(rec, "Dificil");
	
	GuiSetStyle(DEFAULT, TEXT_SIZE, font->baseSize * BUTTON_FONT_SIZE);
	input_rec.height = 60;
	input_rec.width = 600;
	input_rec.x = (width - input_rec.width) / 2; 
	input_rec.y = 195;
	mouse = GetMousePosition();
	edit = mouse.x > input_rec.x &&
	        mouse.x < input_rec.x + input_rec.width &&
	        mouse.y > input_rec.y &&
	        mouse.y < input_rec.y + input_rec.height;
	GuiTextBox(input_rec, input, 30, edit);

	rec.y = (height + rec.y + rec.height) / 2;
	rec.width = MeasureTextEx(*font, "Voltar", font->baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.height = MeasureTextEx(*font, "Voltar", font->baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.y -= rec.height / 2;
	rec.x = width / 2 - rec.width - PANEL_MARGIN_INSIDE;
	if (GuiButton(rec, "Voltar"))
		ret = 0;

	rec.width = MeasureTextEx(*font, "Jogar", font->baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.x = width / 2 + PANEL_MARGIN_INSIDE;
	if (GuiButton(rec, "Jogar"))
		ret = 2; 
	return ret;
}
