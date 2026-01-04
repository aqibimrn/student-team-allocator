// importing relavent libraries
#include "structs.h"
#include "visualizer.h"
#include "splashkit.h"
#include <string>
#include <cmath>
#include <string>

using std::to_string;
using std::vector;

/**
 * creating a button for ui with different colours
 */
void draw_button(const UIButton &btn)
{
    // default colour (blue-ish, tried to match with Student's color)
    color bg = rgb_color(30, 120, 200);

    // if the button is pressed, darken it slightly
    if (btn.pressed)
    {
        bg = rgb_color(20, 90, 160);
    }

    // draw button background
    fill_rectangle(bg, btn.x, btn.y, btn.w, btn.h);
    // draw outline
    draw_rectangle(COLOR_BLACK, btn.x, btn.y, btn.w, btn.h);
    // draw button label
    draw_text(btn.label, COLOR_WHITE, btn.x + 10, btn.y + 10);
}

/**
 * a function to check if the mouse cursor is in the button or not
 */
// a function to check if the mouse cursor is in the button or not
bool point_in_button(float mx, float my, const UIButton &btn)
{
    // return true if mouse coordinates are within the button rectangle
    return ((mx >= btn.x) && (mx <= btn.x + btn.w) && (my >= btn.y) && (my <= btn.y + btn.h));
}

/**
 * a procedure to layout the order of buttons of menu
 */
void layout_buttons(UIContext &ctx, float win_w, float win_h)
{
    // clear any previously existing buttons
    ctx.buttons.clear();

    // starting position for the first button
    float left = 12.0;
    float top = 72.0;
    // size of button
    float bw = 180.0;
    float bh = 36.0;
    // space between buttons
    float gap = 12.0;

    // vector to store label for each button (in order)
    vector<string> labels = {
        "Load CSV", "Compute Scores", "Allocate",
        "Fix Leaders", "Suggest", "Apply Top",
        "View Teams", "Quit"};

    // for loop to create buttons
    for (int i = 0; i < labels.size(); i++)
    {
        UIButton b;
        b.x = left;
        b.y = top + i * (bh + gap); // this is supposed to move it down each time
        b.w = bw;
        b.h = bh;
        b.label = labels[i];
        b.pressed = false;
        ctx.buttons.push_back(b); // add to the UI context's button list
    }
}

/**
 * function to draw team card
 */
void draw_team_card(const team &t, float x, float y, float w, float h)
{
    // light grey background for team card
    color card_bg = rgb_color(245, 248, 252);
    fill_rectangle(card_bg, x, y, w, h);
    // outline around the card
    draw_rectangle(color_black(), x, y, w, h);

    // team title and score displayed at top left of the card
    draw_text("Team " + to_string(t.id), COLOR_BLACK, x + 8, y + 8);
    draw_text("Total: " + to_string(t.total_score), COLOR_BLACK, x + 8, y + 28);

    // color indicator for if a team has a leader or not (red if not, green if yes)
    color indicator;

    if (t.hasLeader)
    {
        indicator = COLOR_GREEN;
    }
    else
    {
        indicator = COLOR_RED;
    }

    fill_rectangle(indicator, x + w - 28, y + 12, 16, 16);

    // draw each student below header
    float rowY = y + 48;
    float chip_h = 26;
    for (int i = 0; i < t.members.size(); i++)
    {
        const student &s = t.members[i];
        // draw rectangle for each student chip
        fill_rectangle(color_white(), x + 8, rowY, w - 16, chip_h);
        draw_rectangle(rgb_color(220, 220, 220), x + 8, rowY, w - 16, chip_h);
        // write student's name and score inside the chip

        // show name and leadership tag
        std::string name_label = s.name + " (Avg Score: " + to_string(s.student_score) + ")";

        // if the student's leadership value meets the threshold, show tag in green
        if (s.leadership >= LEADER_THRESHOLD)
        {
            name_label += " (eligible leader)";
            draw_text(name_label, rgb_color(0, 150, 0), x + 12, rowY + 6);
        }
        else
        {
            draw_text(name_label, color_black(), x + 12, rowY + 6);
        }

        // move down for next student
        rowY += chip_h + 6;

        // stop drawing if we run out of space inside the card
        if (rowY > y + h - 16)
        {
            break;
        }
    }
}

/**
 * draw all the team cards in the grid and handles scrolling
 */
void draw_teams_grid(const vector<team> &teams, float area_x, float scrolled_y_start, float area_w, float area_h, float fixed_area_y)
{
    // error handling
    if (teams.empty())
    {
        draw_text("No teams to display", COLOR_BLACK, area_x + 20, fixed_area_y + 20);
        return;
    }

    // declare constants (visiable area of the screen)
    const float visible_area_top = fixed_area_y;
    const float visible_area_bottom = fixed_area_y + area_h;

    // consants
    const int per_row = 2;
    const float spacing = 12.0;
    const float chip_h = 26.0;
    const float header_h = 48.0;
    const float bottom_padding = 16.0;
    const float max_card_h = 400.0;

    // calculation of width of each card so 2 are fit per row
    float card_w = (area_w - (per_row + 1) * spacing) / per_row;

    // number of rows needed
    int num_teams = teams.size();
    int num_rows = (num_teams + per_row - 1) / per_row;

    // compute height of each card and max height of row
    std::vector<float> card_heights(num_teams, 0.0);
    std::vector<float> row_max_height(num_rows, 0.0);

    for (int idx = 0; idx < num_teams; ++idx)
    {
        int member_count = teams[idx].members.size();

        // compute height = header + one student + bottom padding
        float h = header_h + (member_count * (chip_h + 6.0)) + bottom_padding;

        // limit the height if too many members
        if (h > max_card_h)
        {
            h = max_card_h;
        }

        // store height for this team into card_heights array
        card_heights[idx] = h;

        // update maximum height for the row (which this team belongs to)
        int row = idx / per_row;

        if (h > row_max_height[row])
        {
            row_max_height[row] = h;
        }
    }

    // drawing each row of team cards

    float y_cursor = scrolled_y_start + spacing;

    // goes row across row by going column across column
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < per_row; col++)
        {
            int idx = row * per_row + col;

            // check if all teams are drawn, and if yes then stop
            if (idx >= num_teams)
            {
                break;
            }

            // calculation of x coordinate for this card
            float x = area_x + spacing + col * (card_w + spacing);
            float card_h = card_heights[idx];

            // calculate top and bottom y axis positions for tihs card
            float card_top_y = y_cursor;
            float card_bottom_y = y_cursor + card_h;

            // draw the cards that are visible within the window area
            if (card_bottom_y > visible_area_top && card_top_y < visible_area_bottom)
            {
                draw_team_card(teams[idx], x, card_top_y, card_w, card_h);
            }
        }

        // move y cursor down by the tallest card in this row + the spacing
        y_cursor += row_max_height[row] + spacing;
    }
}
