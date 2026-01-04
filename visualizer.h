#pragma once
#include "structs.h"
#include "ui.h"
#include <vector>
#include <sstream>


void draw_button(const UIButton &btn);

bool point_in_button(float mx, float my, const UIButton &btn);

void layout_buttons(UIContext &ctx, float win_w, float win_h);

void draw_team_card(const team &t, float x, float y, float w, float h);

void draw_teams_grid(const vector<team> &teams, float area_x, float scrolled_y_start, float area_w, float area_h, float fixed_area_y);
