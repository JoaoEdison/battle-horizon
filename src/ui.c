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
#include "../lib/arrays/array.c"
#include "defs.h"
#include "translate.c"
#define RAYGUI_IMPLEMENTATION
#include "raygui.c"

#define PADDING 6

#define MAX_NAME_LEN 14

Font font;
Sound ui_sound;

typedef struct {
    char name[MAX_NAME_LEN];
    int day, month, year, score, seconds;
} score_entry;

array_dynamic scores_easy, scores_hard;

struct {
    int life, state, distance, score;
    int total_shots, total_enemies_destroyed;
    float prev_time, time;
} game_state = { 0 };

char name[MAX_NAME_LEN] = T_M_NAME;

bool easy_map = true;

int ui_screen_width, ui_screen_height;
int title_size = 12, about_size = 4;
int button_font_size = 5, between_buttons_size = 100;
int first_component_y = 195, post_header = 65, width_field_name = 600;
int panel_margin_top = 300, panel_margin_botton = 180, panel_margin_outside = 100;
int win_message_size = 8, step_size = 100, width_life = 20;
int score_size = 2, ratio_width_score_column = 16, spacing = 4;

void config_layout(width, height)
{
    ui_screen_width = width;
    ui_screen_height = height;
    font = LoadFont("data/font/setback.png");
    GuiSetFont(font);
    if (ui_screen_width < 1280) {
        title_size = 5;
        about_size = 2;
        button_font_size = 2;
        between_buttons_size = 60;
        first_component_y = 100;
        raygui_windowbox_statusbar_height = 30;
        post_header = 35;
        width_field_name = 300;
        panel_margin_top = 150;
        panel_margin_outside = 50;
        panel_margin_botton = 90;
        score_size = 1;
        win_message_size = 4;
        step_size = 60;
        ratio_width_score_column = 12;
        spacing = 2;
    } else if (ui_screen_width < 1920) {
        title_size = 9;
        about_size = 3;
        button_font_size = 3;
        between_buttons_size = 75;
        panel_margin_outside = 60;
        panel_margin_top = 210;
        panel_margin_botton = 140;
        ratio_width_score_column = 12;
        raygui_windowbox_statusbar_height = 50;
        first_component_y = 150;
    }
    GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize * button_font_size);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);
    GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
}

/*ordered by score, date, name and seconds*/
sort_scores(x, y)
void *x, *y;
{
    score_entry *ptrx;
    score_entry *ptry;
    int sub;
    
    ptrx = (score_entry*)x;
    ptry = (score_entry*)y;
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
    score_entry *new_score;
    struct tm *ptrtime;
    array_dynamic *scorelist;
    
    scorelist = easy_map? &scores_easy : &scores_hard;
    new_score = (score_entry*) ARRAY_LAST_SPACE_PTR(scorelist);
    scorelist->nmemb++;
    strcpy(new_score->name, name);
    new_score->score = game_state.score;
    new_score->seconds = game_state.time;
    time(&time_raw);
    ptrtime = localtime(&time_raw);
    new_score->day = ptrtime->tm_mday; 
    new_score->month = ptrtime->tm_mon + 1;
    new_score->year = ptrtime->tm_year - 100;
    qsort(scorelist->base, scorelist->nmemb, scorelist->size, sort_scores);
}

draw_ui()
{
    void unload_map();
    float now;
    char time_stamp[8];
    
    if (!game_state.state) {
        DrawRectangle(1, ui_screen_height/2 - (MAX_LIFE * step_size + 2*PADDING) / 2, width_life + 2*PADDING,
                (MAX_LIFE * step_size + 2*PADDING), GRAY);
        DrawRectangle(PADDING+1, ui_screen_height/2 - game_state.life * step_size / 2, width_life, game_state.life * step_size, RED);
        
        DrawRectangle(ui_screen_width - width_life - 2 * PADDING - 1, ui_screen_height/2 - (MAX_LIFE * step_size + 2*PADDING) / 2, width_life + 2*PADDING,
                (MAX_LIFE * step_size + 2*PADDING), GRAY);
        Rectangle rec = {
            ui_screen_width-PADDING-1,
            ui_screen_height/2 + MAX_LIFE * step_size / 2,
            width_life,
            game_state.distance / ARRIVAL_DIST * step_size * MAX_LIFE
        };
        DrawRectanglePro(rec, (Vector2){0.0f, 0.0f}, 180.0f, ORANGE);
        
        now = DEADLINE_SECS + game_state.time;
        now -= GetTime();
        sprintf(time_stamp, "%02d:%02d", (int)(now) / 60, (int)(now) % 60);
        DrawTextEx(font, time_stamp, (Vector2){
                ui_screen_width/2 - MeasureTextEx(font, time_stamp, font.baseSize * win_message_size, spacing).x/2,
                MeasureTextEx(font, time_stamp, font.baseSize * win_message_size, spacing).y/2
                }, font.baseSize * win_message_size, spacing, YELLOW);
    } else if (game_state.state > 0) {
        now = GetTime();
        if (game_state.prev_time == 0.0f)
            game_state.prev_time = now;
        else if (game_state.prev_time + 3.0f <= now)
            return 1;
        DrawTextEx(font, t_m_win, (Vector2){
                ui_screen_width/2 - MeasureTextEx(font, t_m_win, font.baseSize * win_message_size, spacing).x/2,
                ui_screen_height/2 - MeasureTextEx(font, t_m_win, font.baseSize * win_message_size, spacing).y/2
                }, font.baseSize * win_message_size, spacing, GREEN);
    } else {
        now = GetTime();
        if (game_state.prev_time == 0.0f)
            game_state.prev_time = now;
        else if (game_state.prev_time + 3.0f <= now)
            return 1;
        DrawTextEx(font, t_m_lost, (Vector2){
                ui_screen_width/2 - MeasureTextEx(font, t_m_lost, font.baseSize * win_message_size, spacing).x/2,
                ui_screen_height/2 - MeasureTextEx(font, t_m_lost, font.baseSize * win_message_size, spacing).y/2
                }, font.baseSize * win_message_size, spacing, RED);
    }
    return 0;
}

#define BUTTON_PADDING_SIDES 20
#define BUTTON_PADDING_VERT 10

draw_menu()
{
    int ret;
       
    ret = 0;
    Rectangle rec;
    rec.width = MeasureTextEx(font, t_b_play, font.baseSize * button_font_size, spacing).x + BUTTON_PADDING_SIDES;
    rec.height = MeasureTextEx(font, t_b_play, font.baseSize * button_font_size, spacing).y + BUTTON_PADDING_VERT;
    rec.x = (ui_screen_width - rec.width) / 2;
    rec.y = ui_screen_height / 2 - 150;
    if (GuiButton(rec, t_b_play))
        ret = 1;
    rec.y += between_buttons_size;
    if (GuiButton(rec, t_b_about))
        ret = 3;
    rec.y += between_buttons_size;
    if (GuiButton(rec, t_b_exit))
        ret = 4;
    if (ret)
        PlaySound(ui_sound);
    return ret;
}

#define PRINT_BUFFER \
        DrawTextEx(font, buffer, (Vector2){ \
                x_left + curr_inc + (inc - MeasureTextEx(font, buffer, font.baseSize * score_size, spacing).x) / 2, \
                y_begin \
                }, font.baseSize * score_size, spacing, GetColor(GuiGetStyle(STATUSBAR, BASE)));

void draw_scores(v, x_left, x_right, y_begin)
array_dynamic *v;
{
    score_entry *ptrsc;
    char buffer[50];
    int inc, curr_inc, i;

    for (i=0; i < v->nmemb; i++) {    
        ptrsc = (score_entry*) ARRAY_AT_PTR(v, i);
        curr_inc = 0;
        inc = (x_right - x_left) / ratio_width_score_column;
        sprintf(buffer, "%d", ptrsc->score);
        PRINT_BUFFER
        curr_inc = (x_right - x_left) / ratio_width_score_column;

        inc = (x_right - x_left) / 2;
        sprintf(buffer, "%s", ptrsc->name);
        PRINT_BUFFER
        curr_inc += (x_right - x_left) / 2;
        
        inc = (x_right - x_left) / 5;
        sprintf(buffer, "%02d/%02d/%02d", ptrsc->day, ptrsc->month, ptrsc->year);
        PRINT_BUFFER
        curr_inc += (x_right - x_left) / 5;
        
        inc = (x_right - x_left) * 0.2375f;
        sprintf(buffer, "%02d:%02d", ptrsc->seconds / 60, ptrsc->seconds % 60);
        PRINT_BUFFER
        
        y_begin += MeasureTextEx(font, buffer, font.baseSize * score_size, spacing).y;
    }
}

#define PANEL_MARGIN_INSIDE 20

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

draw_play()
{
    Rectangle rec;
    int ret = 1;
    bool edit;
    
    rec.width = ui_screen_width / 2 - PANEL_MARGIN_INSIDE - panel_margin_outside;
    rec.height = ui_screen_height - panel_margin_top - panel_margin_botton;
    rec.x = panel_margin_outside;
    rec.y = panel_margin_top;
    GuiPanel(rec, t_h_easy);
    draw_scores(&scores_easy, (int)(rec.x), (int)(rec.x + rec.width), (int)(rec.y) + post_header);
    if (easy_map)
        DrawRectangleLinesEx(rec, 3.0f, SKYBLUE);
    if (mouse_in_rectangle(&rec))
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
            IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(ui_sound);
            easy_map = true;
        }
    
    rec.x = ui_screen_width / 2 + PANEL_MARGIN_INSIDE;
    GuiPanel(rec, t_h_hard);
    draw_scores(&scores_hard, (int)(rec.x), (int)(rec.x + rec.width), (int)(rec.y) + post_header);
    if (!easy_map)
        DrawRectangleLinesEx(rec, 3.0f, RED);
    if (mouse_in_rectangle(&rec))
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
            IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlaySound(ui_sound);
            easy_map = false;
        }

    rec.y = (ui_screen_height + rec.y + rec.height) / 2;
    rec.width = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).x + BUTTON_PADDING_SIDES;
    rec.height = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).y + BUTTON_PADDING_VERT;
    rec.y -= rec.height / 2;
    rec.x = ui_screen_width / 2 - rec.width - PANEL_MARGIN_INSIDE;
    if (GuiButton(rec, t_b_back))
        ret = 0;

    rec.width = MeasureTextEx(font, t_b_play, font.baseSize * button_font_size, spacing).x + BUTTON_PADDING_SIDES;
    rec.x = ui_screen_width / 2 + PANEL_MARGIN_INSIDE;
    if (GuiButton(rec, t_b_play))
        ret = 2;
    
    rec.height = MeasureTextEx(font, name, font.baseSize * button_font_size, spacing).y + 10;
    rec.width = width_field_name;
    rec.x = (ui_screen_width - rec.width) / 2; 
    rec.y = first_component_y;
    edit = mouse_in_rectangle(&rec);
    GuiTextBox(rec, name, MAX_NAME_LEN, edit);

    if (ret != 1)
        PlaySound(ui_sound);
    return ret;
}

#define DRAW_CELL(PREV_LINE, LINE, POINT) \
    vec2.x = first_column; \
    vec2.y += MeasureTextEx(font, PREV_LINE, size, spacing).y; \
    DrawTextEx(font, LINE, vec2, size, spacing, color); \
    vec2.x = second_colum; \
    DrawTextEx(font, buffer, vec2, size, spacing, color); \
    vec2.x = third_column; \
    DrawTextEx(font, STRINGIZE(POINT), vec2, size, spacing, color);

score_table()
{
    Rectangle rec;
    Vector2 vec2;
    Color color;
    int ret;
    int size;
    int first_column, second_colum, third_column;
    char buffer[4];
    
    ret = 5;
    size = font.baseSize * about_size;
    first_column = panel_margin_outside * 2;
    second_colum = ui_screen_width / 2 + panel_margin_outside;
    third_column = ui_screen_width - first_column - MeasureTextEx(font, t_c_points, size, spacing).x;
    DrawTextEx(font, t_t_log, (Vector2){
            ui_screen_width/2 - MeasureTextEx(font, t_t_log, font.baseSize * title_size, spacing).x/2,
            MeasureTextEx(font, t_t_log, font.baseSize * title_size, spacing).y/4
            }, font.baseSize * title_size, spacing, BLUE);
    color =    GetColor(GuiGetStyle(STATUSBAR, BASE));
    vec2.x = second_colum;
    vec2.y = panel_margin_top;
    DrawTextEx(font, t_c_total, vec2, size, spacing, color);
    vec2.x = third_column;
    DrawTextEx(font, t_c_points, vec2, size, spacing, color);
    
    sprintf(buffer, "%d", game_state.life);
    DRAW_CELL(t_c_total, t_c_life, SCORE_PER_LIFE)
    sprintf(buffer, "%d", game_state.total_enemies_destroyed);
    DRAW_CELL(t_c_life, t_c_enemies, SCORE_PER_ENEMY)
    sprintf(buffer, "%d", (int)game_state.time);
    DRAW_CELL(t_c_enemies, t_c_time, SCORE_PER_SECOND)
    sprintf(buffer, "%d", game_state.total_shots);
    DRAW_CELL(t_c_time, t_c_shots, SCORE_PER_SHOT)
    
    sprintf(buffer, "%d", game_state.score);
    vec2.x = first_column;    
    vec2.y += MeasureTextEx(font, t_c_time, size, spacing).y;
    DrawTextEx(font, t_c_sum, vec2, size, spacing, color);
    vec2.x = third_column;
    DrawTextEx(font, buffer, vec2, size, spacing, color);

    rec.width = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).x + BUTTON_PADDING_SIDES;
    rec.height = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).y + BUTTON_PADDING_VERT;
    rec.x = ui_screen_width / 2  - rec.width - PANEL_MARGIN_INSIDE;
    rec.y = ui_screen_height / 2 + first_component_y;    
    if (GuiButton(rec, t_b_back))
        ret = 6;
    rec.x = ui_screen_width / 2 + PANEL_MARGIN_INSIDE; 
    if (GuiButton(rec, t_b_play))
        ret = 7;
    if (ret != 5) {
        PlaySound(ui_sound);
        if (game_state.state == 1)
            save_score();
    }
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

draw_about()
{
    int ret;
    
    ret = 3;
    Rectangle rec;
    rec.width = ui_screen_width - 2 * panel_margin_outside;
    rec.height = ui_screen_height - first_component_y - panel_margin_botton;
    rec.x = panel_margin_outside;
    rec.y = first_component_y;
    GuiPanel(rec, t_h_about);
    rec.y += post_header;
    rec.x += 5;
    DrawTextBoxedSelectable(font, text_about, rec, font.baseSize * about_size, 1, GetColor(GuiGetStyle(STATUSBAR, BASE)), 0, 0, BLACK, BLACK);
    rec.y -= post_header;
    rec.width = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).x + BUTTON_PADDING_SIDES;
    rec.y = (ui_screen_height + rec.y + rec.height) / 2;
    rec.height = MeasureTextEx(font, t_b_back, font.baseSize * button_font_size, spacing).y + BUTTON_PADDING_VERT;
    rec.y -= rec.height / 2;
    rec.x = (ui_screen_width - rec.width) / 2;
    if (GuiButton(rec, t_b_back))
        ret = 0;
    if (!ret)
        PlaySound(ui_sound);
    return ret;
}
