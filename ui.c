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
#include <time.h>
#include "linked_list.c"
#include "array.c"
#include "defs.h"
#include "translate.c"
#define RAYGUI_IMPLEMENTATION
#include "raygui.c"

#define STEP_SIZE 100
#define PADDING 6
#define WIDTH_LIFE 20
#define SPACING 4
#define WIN_MESSAGE_SIZE 8

#define MAX_NAME_LEN 20

Font font;
Sound ui_sound;

struct score_entry {
	char name[MAX_NAME_LEN];
	unsigned char day, month, year;
	int score, seconds;
};

struct vector_d scores_easy, scores_hard;

struct {
	int life, state, distance, score;
	float prev_time, time;
} game_state = { 0 };

char name[MAX_NAME_LEN] = T_M_NAME;

bool easy_map = true;

/*ordenado por pontuação, data, nome e segundos*/
sort_scores(x, y)
void *x, *y;
{
	struct score_entry *ptrx;
	struct score_entry *ptry;
	int sub;
	
	ptrx = (struct score_entry*)x;
	ptry = (struct score_entry*)y;
	if ((sub = ptry->score - ptrx->score))
		return sub;
	if ((sub = ptry->year - ptrx->year))
		return sub;
	if ((sub = ptry->month - ptrx->month))
		return sub;
	if ((sub = ptry->day - ptrx->day))
		return sub;
	if ((sub = strcmp(ptry->name, ptrx->name)))
		return sub;
	if ((sub = ptry->seconds - ptrx->seconds))
		return sub;
	return 0;
}

static void save_score()
{
	time_t time_raw;
	struct score_entry *new_score;
	struct tm *ptrtime;
	struct vector_d *scorelist;
	
	scorelist = easy_map? &scores_easy : &scores_hard;
	new_score = (struct score_entry*) LAST_SPACE_PTR(scorelist);
	scorelist->nmemb++;
	strcpy(new_score->name, name);
	new_score->score = game_state.score;
	new_score->seconds = DEADLINE_SECS - GetTime() + game_state.time + 3;
	time(&time_raw);
	ptrtime = localtime(&time_raw);
	new_score->day = ptrtime->tm_mday; 
	new_score->month = ptrtime->tm_mon + 1;
	new_score->year = ptrtime->tm_year - 100;
	qsort(scorelist->base, scorelist->nmemb, scorelist->sizememb, sort_scores);
}

draw_ui(width, height)
{
	void unload_map();
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
		DrawTextEx(font, time_stamp, (Vector2){
				width/2 - MeasureTextEx(font, time_stamp, font.baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				MeasureTextEx(font, time_stamp, font.baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font.baseSize * WIN_MESSAGE_SIZE, SPACING, YELLOW);
	} else if (game_state.state > 0) {
		now = GetTime();
		if (game_state.prev_time == 0.0f)
			game_state.prev_time = now;
		else if (game_state.prev_time + 3.0f <= now) {
			game_state.score += game_state.life * SCORE_PER_LIFE;
			game_state.score += (DEADLINE_SECS + game_state.time - GetTime()) * SCORE_PER_SECOND;
			save_score();
			unload_map();
			return 1;
		}
		DrawTextEx(font, t_m_win, (Vector2){
				width/2 - MeasureTextEx(font, t_m_win, font.baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				height/2 - MeasureTextEx(font, t_m_win, font.baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font.baseSize * WIN_MESSAGE_SIZE, SPACING, GREEN);
	} else {
		now = GetTime();
		if (game_state.prev_time == 0.0f)
			game_state.prev_time = now;
		else if (game_state.prev_time + 3.0f <= now) {
			unload_map();
			return 1;
		}
		DrawTextEx(font, t_m_lost, (Vector2){
				width/2 - MeasureTextEx(font, t_m_lost, font.baseSize * WIN_MESSAGE_SIZE, SPACING).x/2,
				height/2 - MeasureTextEx(font, t_m_lost, font.baseSize * WIN_MESSAGE_SIZE, SPACING).y/2
				}, font.baseSize * WIN_MESSAGE_SIZE, SPACING, RED);
	}
	return 0;
}

#define TITLE_SIZE 12
#define BUTTON_FONT_SIZE 5
#define BUTTON_PADDING_SIDES 20
#define BUTTON_PADDING_VERT 10

draw_menu(width, height)
{
	int ret;
       
	ret = 0;
	Rectangle rec;
	GuiSetFont(font);
	GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize * BUTTON_FONT_SIZE);
	rec.width = MeasureTextEx(font, t_m_play, font.baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.height = MeasureTextEx(font, t_m_play, font.baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.x = (width - rec.width) / 2;
	rec.y = height / 2 - 150;
	if (GuiButton(rec, t_m_play))
		ret = 1;
	rec.y += 100;
	if (GuiButton(rec, t_m_about))
		ret = 3;
	rec.y += 100;
	if (GuiButton(rec, t_m_exit))
		ret = 4;
	if (ret)
		PlaySound(ui_sound);
	return ret;
}

#define SCORE_SIZE 2
#define PRINT_BUFFER \
		DrawTextEx(font, buffer, (Vector2){ \
				x_left + curr_inc + (inc - MeasureTextEx(font, buffer, font.baseSize * SCORE_SIZE, SPACING).x) / 2, \
				y_begin \
				}, font.baseSize * SCORE_SIZE, SPACING, GetColor(GuiGetStyle(STATUSBAR, BASE)));

void draw_scores(v, x_left, x_right, y_begin)
struct vector_d *v;
{
	struct score_entry *ptrsc;
	char buffer[50];
	int inc, curr_inc, i;

	for (i=0; i < v->nmemb; i++) {	
		ptrsc = (struct score_entry*) AT_PTR(v, i);
		curr_inc = 0;
		inc = (x_right - x_left) / 16;
		sprintf(buffer, "%d", ptrsc->score);
		PRINT_BUFFER
		curr_inc = (x_right - x_left) / 16;

		inc = (x_right - x_left) / 2;
		sprintf(buffer, "%s", ptrsc->name);
		PRINT_BUFFER
		curr_inc += (x_right - x_left) / 2;
		
		inc = (x_right - x_left) / 5;
		sprintf(buffer, "%02hhd/%02hhd/%02hhd", ptrsc->day, ptrsc->month, ptrsc->year);
		PRINT_BUFFER
		curr_inc += (x_right - x_left) / 5;
		
		inc = (x_right - x_left) * 0.2375f;
		sprintf(buffer, "%02hhd:%02hhd", ptrsc->seconds / 60, ptrsc->seconds % 60);
		PRINT_BUFFER
		
		y_begin += MeasureTextEx(font, buffer, font.baseSize * SCORE_SIZE, SPACING).y;
	}
}

#define PANEL_MARGIN_OUTSIDE 100
#define PANEL_MARGIN_INSIDE 20
#define PANEL_MARGIN_TOP 300
#define PANEL_MARGIN_BOTTON 180
#define PANEL_HEADER_SIZE 3
#define FIRST_COMPONENT_Y 195

mouse_in_rectangle(rec)
Rectangle *rec;
{
	Vector2 mouse;
	
	mouse = GetMousePosition();
	return (mouse.x > rec->x &&
	        mouse.x < rec->x + rec->width &&
	        mouse.y > rec->y &&
	        mouse.y < rec->y + rec->height);
}

draw_play(width, height)
{
	Rectangle rec;
	int ret;
	bool edit;
	
	ret = 1;
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);
	GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	rec.width = width / 2 - PANEL_MARGIN_INSIDE - PANEL_MARGIN_OUTSIDE;
	rec.height = height - PANEL_MARGIN_TOP - PANEL_MARGIN_BOTTON;
	rec.x = PANEL_MARGIN_OUTSIDE;
	rec.y = PANEL_MARGIN_TOP;
	GuiPanel(rec, t_m_easy);
	draw_scores(&scores_easy, (int)(rec.x), (int)(rec.x + rec.width), (int)(rec.y) + 65);
	if (easy_map)
		DrawRectangleLinesEx(rec, 3.0f, SKYBLUE);
	if (mouse_in_rectangle(&rec))
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
		    IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
		    IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			PlaySound(ui_sound);
			easy_map = true;
		}
	
	rec.x = width / 2 + PANEL_MARGIN_INSIDE;
	GuiPanel(rec, t_m_hard);
	draw_scores(&scores_hard, (int)(rec.x), (int)(rec.x + rec.width), (int)(rec.y) + 65);
	if (!easy_map)
		DrawRectangleLinesEx(rec, 3.0f, RED);
	if (mouse_in_rectangle(&rec))
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
		    IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
		    IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			PlaySound(ui_sound);
			easy_map = false;
		}

	rec.y = (height + rec.y + rec.height) / 2;
	rec.width = MeasureTextEx(font, t_m_back, font.baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.height = MeasureTextEx(font, t_m_back, font.baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.y -= rec.height / 2;
	rec.x = width / 2 - rec.width - PANEL_MARGIN_INSIDE;
	if (GuiButton(rec, t_m_back))
		ret = 0;

	rec.width = MeasureTextEx(font, t_m_play, font.baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.x = width / 2 + PANEL_MARGIN_INSIDE;
	if (GuiButton(rec, t_m_play))
		ret = 2;
	
	GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize * BUTTON_FONT_SIZE);
	rec.height = 60;
	rec.width = 600;
	rec.x = (width - rec.width) / 2; 
	rec.y = FIRST_COMPONENT_Y;
	edit = mouse_in_rectangle(&rec);
	GuiTextBox(rec, name, 30, edit);

	if (ret != 1)
		PlaySound(ui_sound);
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

draw_about(width, height)
{
	int ret;
	
	ret = 3;
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);
	GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
	Rectangle rec;
	rec.width = width - 2 * PANEL_MARGIN_OUTSIDE;
	rec.height = height - FIRST_COMPONENT_Y - PANEL_MARGIN_BOTTON;
	rec.x = PANEL_MARGIN_OUTSIDE;
	rec.y = FIRST_COMPONENT_Y;
	GuiPanel(rec, t_m_about_header);
	rec.y += 65;
	rec.x += 5;
	DrawTextBoxedSelectable(font, text_about, rec, font.baseSize * 4, 1, GetColor(GuiGetStyle(STATUSBAR, BASE)), 0, 0, BLACK, BLACK);
	rec.y -= 65;
	rec.width = MeasureTextEx(font, t_m_back, font.baseSize * BUTTON_FONT_SIZE, SPACING).x + BUTTON_PADDING_SIDES;
	rec.y = (height + rec.y + rec.height) / 2;
	rec.height = MeasureTextEx(font, t_m_back, font.baseSize * BUTTON_FONT_SIZE, SPACING).y + BUTTON_PADDING_VERT;
	rec.y -= rec.height / 2;
	rec.x = (width - rec.width) / 2;
	if (GuiButton(rec, t_m_back))
		ret = 0;
	if (!ret)
		PlaySound(ui_sound);
	return ret;
}
