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

struct {
	int life, state, distance, score;
	float prev_time, time;
} game_state = { 0 };

draw_ui(width, height, font)
Font *font;
{
	float now;
	char time_stamp[8];
	
	if (!game_state.state) {
		DrawRectangle(1, height/2 - (MAX_LIFE * STEP_SIZE + 2*PADDING) / 2, WIDTH_LIFE + 2*PADDING,
				(MAX_LIFE * STEP_SIZE + 2*PADDING), GRAY);
		DrawRectangle(PADDING+1, height/2 - game_state.life * STEP_SIZE / 2, WIDTH_LIFE, game_state.life * STEP_SIZE, RED);
		
		DrawRectangle(width - WIDTH_LIFE - 2 * PADDING - 1, height/2 - (MAX_LIFE * STEP_SIZE + 2*PADDING) / 2, WIDTH_LIFE + 2*PADDING,
				(MAX_LIFE * STEP_SIZE + 2*PADDING), GRAY);
		Rectangle rec = {
			width-PADDING-1,
			height/2 + MAX_LIFE * STEP_SIZE / 2,
			WIDTH_LIFE,
			game_state.distance / ARRIVAL_DIST * STEP_SIZE * MAX_LIFE
		};
		DrawRectanglePro(rec, (Vector2){0.0f, 0.0f}, 180.0f, ORANGE);
		
		now = DEADLINE_SECS + game_state.time;
		now -= GetTime();
		sprintf(time_stamp, "%02hhd:%02hhd", (int)(now) / 60, (int)(now) % 60);
		DrawTextEx(*font, time_stamp, (Vector2){
				width/2 - MeasureTextEx(*font, time_stamp, font->baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				MeasureTextEx(*font, time_stamp, font->baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font->baseSize * WIN_MESSAGE_SIZE, SPACING, YELLOW);
	} else if (game_state.state > 0) {
		now = GetTime();
		if (game_state.prev_time == 0.0f)
			game_state.prev_time = now;
		else if (game_state.prev_time + 3.0f <= now) {
			game_state.score += game_state.life * 20;
			game_state.score += (DEADLINE_SECS + game_state.time - GetTime()) * 3;
			printf("%d\n", game_state.score);
			return 1;
		}
		DrawTextEx(*font, "VOCE VENCEU", (Vector2){
				width/2 - MeasureTextEx(*font, "VOCE VENCEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				height/2 - MeasureTextEx(*font, "VOCE VENCEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font->baseSize * WIN_MESSAGE_SIZE, SPACING, GREEN);
	} else {
		now = GetTime();
		if (game_state.prev_time == 0.0f)
			game_state.prev_time = now;
		else if (game_state.prev_time + 3.0f <= now)
			return 1;
		DrawTextEx(*font, "VOCE PERDEU", (Vector2){
				width/2 - MeasureTextEx(*font, "VOCE PERDEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				height/2 - MeasureTextEx(*font, "VOCE PERDEU", font->baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font->baseSize * WIN_MESSAGE_SIZE, SPACING, RED);
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
		ret = 3;
	rec.y += 100;
	if (GuiButton(rec, "Sair"))
		ret = 4;
	return ret;
}

#define PANEL_MARGIN_OUTSIDE 100
#define PANEL_MARGIN_INSIDE 20
#define PANEL_MARGIN_TOP 300
#define PANEL_MARGIN_BOTTON 180
#define PANEL_HEADER_SIZE 3
#define FIRST_COMPONENT_Y 195

draw_play(width, height, font, name)
Font *font;
char name[];
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
	input_rec.y = FIRST_COMPONENT_Y;
	mouse = GetMousePosition();
	edit = mouse.x > input_rec.x &&
	        mouse.x < input_rec.x + input_rec.width &&
	        mouse.y > input_rec.y &&
	        mouse.y < input_rec.y + input_rec.height;
	GuiTextBox(input_rec, name, 30, edit);

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

/* Copyright (c) 2018-2023 Vlad Adrian (@demizdor) and Ramon Santamaria (@raysan5) */
// Draw text using font inside rectangle limits with support for text selection
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop
    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw
    float scaleFactor = fontSize/(float)font.baseSize;     // Character rectangle scaling factor
    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = MEASURE_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            // When text overflows rectangle height limit, just stop drawing
            if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

            // Draw selection background
            bool isGlyphSelected = false;
            if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
            {
                DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                isGlyphSelected = true;
            }

            // Draw current character glyph
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX, rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
            }

            if (i == endLine)
            {
                textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}

static char text_about[] = "Spacecraft eh um jogo de batalha espacial desenvolvido no motor de jogo Raylib.";

draw_about(width, height, font)
Font *font;
{
	int ret = 3;

	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);
	GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	Rectangle rec;
	rec.width = width - 2 * PANEL_MARGIN_OUTSIDE;
	rec.height = height - FIRST_COMPONENT_Y - PANEL_MARGIN_BOTTON;
	rec.x = PANEL_MARGIN_OUTSIDE;
	rec.y = FIRST_COMPONENT_Y;
	GuiPanel(rec, "Sobre o Jogo");
	rec.y += 65;
	rec.x += 5;
	DrawTextBoxedSelectable(*font, text_about, rec, font->baseSize * 4, 1, GetColor(GuiGetStyle(STATUSBAR, BASE)), 0, 0, BLACK, BLACK);
	rec.y -= 65;
	rec.width = MeasureTextEx(*font, "Voltar", font->baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.y = (height + rec.y + rec.height) / 2;
	rec.height = MeasureTextEx(*font, "Voltar", font->baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.y -= rec.height / 2;
	rec.x = (width - rec.width) / 2;
	if (GuiButton(rec, "Voltar"))
		ret = 0;
	return ret;
}
