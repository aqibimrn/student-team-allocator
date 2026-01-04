// importing libraries
#include "structs.h"
#include "ui.h"
#include "visualizer.h"
#include "io.h"
#include "scoring.h"
#include "allocator.h"
#include "splashkit.h"
#include "utilities.h"
#include <sstream>
#include "optimizer.h"

#include <string>

using std::string;
using std::vector;

/**
 * initializing the UI context, with the items that are needed
 */
void ui_init(UIContext &ctx)
{
    // initialize variables
    ctx.students.clear();
    ctx.teams.clear();
    ctx.suggestions.clear();
    ctx.chosenSuggestionIndex = -1;
    ctx.buttons.clear();
    ctx.running = true;
    ctx.suggestions_locked = false;

    // the size of window is needed to ensure the button layout size. I will be going for 1280x720
    layout_buttons(ctx, 1280.0, 720.0);
}

/**
 * basic cleanup if i ever need it
 */
void ui_cleanup(UIContext &ctx)
{
    ctx.buttons.clear();
    ctx.running = false;
}

/**
 * a function that would wrap long text into multiple lines so it fits nicely on screen
 */
std::vector<std::string> wrap_text(const std::string &text, int max_chars)
{
    // initializing vairables
    std::vector<std::string> lines;
    std::string word;
    std::string line;

    // using string stream to split inputs into words
    std::istringstream iss(text);
    while (iss >> word)
    {
        // error handling where if the line is empty it would just start it with the word
        if (line.empty())
        {
            line = word;
        }

        // have to make sure its below max_chars
        else if (line.size() + 1 + word.size() <= max_chars)
        {
            line += " " + word;
        }

        // otherwise push current line and start a new one
        else
        {
            lines.push_back(line);
            line = word;
        }
    }

    if (!line.empty())
    {
        lines.push_back(line);
    }

    return lines;
}

/**
 * this is going to be my main function that would run the UI. it open the window and handles inputs and draws everything on screen
 */
void ui_run(UIContext &ctx)
{
    // fixing window size. I will be going for 720p!
    const int WIN_W = 1280;
    const int WIN_H = 720;

    // Open window
    open_window("Student Team Builder (GUI)", WIN_W, WIN_H);
    
    // i loaded in a custom font
    load_font("input", "arial.ttf");

    // initialize inputs
    ctx.reading_csv = false;
    ctx.reading_teams = false;
    ctx.input_rect = rectangle_from(230.0, 50.0, 300.0, 30.0);
    ctx.current_input = "";
    ctx.status_message = "";

    // Initialize scrolling state
    ctx.scroll_offset_y = 0.0; // how much scrolled
    ctx.max_scroll_y = 0.0; // max amt which can be scrolled

    // call function and position the buttons
    layout_buttons(ctx, WIN_W, WIN_H);

    // Main UI loop
    while (ctx.running && !window_close_requested("Student Team Builder (GUI)"))
    {
        process_events();

        // Define the area for where teams will be displayed
        const float AREA_X_START = 220.0;
        const float AREA_Y_TOP = 48.0;
        const float STATS_PANEL_W = 200.0;
        const float AREA_W = WIN_W - AREA_X_START - STATS_PANEL_W - 40.0;
        const float AREA_H = WIN_H - AREA_Y_TOP - 40.0;

        // function from utilities to calc how tall the list is
        float content_height = calculate_total_teams_display_height(ctx.teams);

        // dont scroll if max
        ctx.max_scroll_y = fmax(0, content_height - AREA_H);

        // check mouse wheel scroll and process it
        if (mouse_wheel_scroll().y != 0)
        {
            // i can change speed of scrolling here
            float scroll_speed = 40.0;
            // check direction of scroll
            ctx.scroll_offset_y -= mouse_wheel_scroll().y * scroll_speed;
        }

        // indicate how far the scrolling can go
        ctx.scroll_offset_y = fmax(0, ctx.scroll_offset_y);
        ctx.scroll_offset_y = fmin(ctx.max_scroll_y, ctx.scroll_offset_y);

        // handle inputs for csv and teams
        if (ctx.reading_csv || ctx.reading_teams)
        {
        }
        if (!reading_text())
        {
            // eerror handling
            if (text_entry_cancelled())
            {
                ctx.status_message = "Input cancelled.";
            }

            else
            {
                // get what user typed
                std::string input = text_input();

                // csv file input handling
                if (ctx.reading_csv)
                {
                    std::string name = input;

                    // error handling
                    if (name == "")
                    {
                        name = "sample_data.csv";
                    }

                    // load students from csv
                    std::vector<student> loaded = load_students_from_csv(name);

                    // error handling for if no students were loaded
                    if (loaded.empty())
                    {
                        ctx.status_message = "No students loaded from: " + name;
                        write_line(ctx.status_message);
                    }

                    else
                    {
                        ctx.students = loaded;
                        ctx.suggestions_locked = false;

                        ctx.teams.clear();
                        ctx.status_message = ("Loaded " + std::to_string(ctx.students.size()) + " students from " + name);
                        write_line(ctx.status_message);
                    }

                    ctx.reading_csv = false;
                }

                // teams input handling
                else if (ctx.reading_teams)
                {
                    int numTeams = 0;

                    // using catch as i learnt in the programmers.guide to avoid crashes
                    try
                    {
                        numTeams = std::stoi(input);
                    }
                    catch (...)
                    {
                        numTeams = 0;
                    }

                    // error handling if invalid input
                    if (numTeams <= 0)
                    {
                        ctx.status_message = ("Invalid number of teams: " + input + "");
                    }
                    
                    else if (ctx.students.empty())
                    {
                        ctx.status_message = "Cannot allocate teams: no students loaded.";
                    }

                    else
                    {
                        // ensure scores computed
                        bool need_compute = false;

                        // loop to check if scores were computed
                        for (int i = 0; i < ctx.students.size(); i++)
                        {
                            if (ctx.students[i].student_score == 0)
                            {
                                need_compute = true;
                                break;
                            }
                        }

                        if (need_compute)
                        {
                            compute_scores_for_all(ctx.students);
                            ctx.status_message = "Scores computed before team allocation.";
                        }

                        // making the teams!
                        ctx.teams = allocate_teams(ctx.students, numTeams);

                        ensure_leader_present(ctx.teams);

                        ctx.suggestions_locked = false;

                        ctx.status_message = "Allocated " + std::to_string(numTeams) + " teams successfully.";
                    }

                    ctx.reading_teams = false;
                }
            }
        }

        // when mouse clicked
        if (!ctx.reading_csv && !ctx.reading_teams && mouse_clicked(LEFT_BUTTON))
        {
            // position of mouse
            float mx = mouse_x();
            float my = mouse_y();

            // find clicked button index
            int clicked_idx = -1;

            // loop thru all buttons to check which one was clicked by the mouse
            for (int i = 0; i < ctx.buttons.size(); i++)
            {
                if (point_in_button(mx, my, ctx.buttons[i]))
                {
                    clicked_idx = i;
                    break;
                }
            }

            if (clicked_idx != -1)
            {
                // reset the pressed visual for all buttons
                for (int k = 0; k < ctx.buttons.size(); k++)
                {
                    ctx.buttons[k].pressed = false;
                }

                // only mark this one as pressed
                ctx.buttons[clicked_idx].pressed = true;

                // check what the label is on the button for running the relavent command
                std::string label = ctx.buttons[clicked_idx].label;

                // if user clicked on load csv
                if (label == "Load CSV")
                {
                    ctx.input_rect = rectangle_from(12.0, 492.0, 180.0, 36.0);
                    ctx.current_input.clear();

                    // initalize the box for typing
                    start_reading_text(ctx.input_rect);
                    
                    // boolean for waiting for the csv filename from user
                    ctx.reading_csv = true;
                    ctx.status_message = ("Type CSV filename and press Enter (Esc to cancel).");
                }

                // if user clicked on compute scores
                else if (label == "Compute Scores")
                {
                    // error handling with a warning message
                    if (ctx.students.empty())
                    {
                        ctx.status_message = "Compute Scores: no students loaded.";
                        write_line(ctx.status_message);
                    }

                    else
                    {
                        compute_scores_for_all(ctx.students);
                        ctx.status_message = "Computed scores for " + std::to_string(ctx.students.size()) + " students.";
                        write_line(ctx.status_message);
                    }
                }

                // if user clicked on allocate
                else if (label == "Allocate")
                {
                    if (ctx.students.empty())
                    {
                        ctx.status_message = "Allocate: no students loaded.";
                    }

                    else
                    {
                        ctx.input_rect = rectangle_from(12.0, 492.0, 180.0, 36.0);
                        ctx.current_input.clear();
                        
                        // open textbox
                        start_reading_text(ctx.input_rect);
                        ctx.reading_teams = true;
                        ctx.status_message = ("Type number of teams and press Enter.");
                    }
                }

                // button for fix leaders (without any sophisticated algorithm)
                else if (label == "Fix Leaders")
                {
                    if (ctx.teams.empty())
                    {
                        ctx.status_message = "Allocate teams first.";
                        write_line("Allocate teams first.");
                    }

                    else
                    {
                        // ensure every team has a leader
                        ensure_leader_present(ctx.teams);
                        ctx.suggestions_locked = false;

                        // check if every team still doesn't have a leader

                        bool anyMissingLeader = false;

                        for (int i = 0; i < ctx.teams.size(); i++)
                        {
                            if (!ctx.teams[i].hasLeader)
                            {
                                anyMissingLeader = true;
                                break;
                            }
                        }

                        if (anyMissingLeader)
                        {
                            ctx.status_message = "WARNING: not all teams have leaders due to unavailability of leaders.";
                        }

                        else
                        {
                            ctx.status_message = "All teams have leaders.";
                        }

                        // just as a safe check to output to the cli (maybe for debugging :D)
                        write_line(ctx.status_message);
                    }
                }

                // button for if user clicks suggest (advanced algorithm)
                else if (label == "Suggest")
                {
                    // compute suggestions
                    ctx.suggestions.clear();

                    // only show 10 suggestions max
                    const int MAX_SUGGS = 10;

                    // Suggest button handler
                    if (ctx.suggestions_locked)
                    {
                        ctx.status_message = "Suggestions locked (teams were modified by Apply Top). Change teams to re-enable suggestions.";
                    }

                    else
                    {
                        generate_swap_suggestions(ctx.teams, MAX_SUGGS, ctx.suggestions);

                        if (ctx.suggestions.empty())
                        {
                            ctx.status_message = "No swap suggestions available.";
                        }
                        else
                        {
                            ctx.chosenSuggestionIndex = 0;
                            ctx.status_message = "Generated " + std::to_string(ctx.suggestions.size()) + " suggestions. Use Apply Top to apply best one.";
                        }
                    }
                }

                // button for apply top (apply advanced algorithm suggestions)
                else if (label == "Apply Top")
                {
                    if (ctx.suggestions.empty())
                    {
                        ctx.status_message = ("No suggestions available. Click Suggest first.");
                    }

                    else
                    {
                        int idx = ctx.chosenSuggestionIndex;

                        // crash/error handling againa
                        if (idx < 0 || idx >= ctx.suggestions.size())
                        {
                            idx = 0;
                        }

                        SwapSuggestion s = ctx.suggestions[idx];


                        // Do the actual swap now:
                        student temp = ctx.teams[s.teamA].members[s.idxA];

                        ctx.teams[s.teamA].members[s.idxA] = ctx.teams[s.teamB].members[s.idxB];

                        ctx.teams[s.teamB].members[s.idxB] = temp;

                        // recompute affected team stats
                        recompute_team_stats(ctx.teams[s.teamA]);
                        recompute_team_stats(ctx.teams[s.teamB]);

                        ctx.suggestions.clear();
                        
                        // prevent new suggestions until teams are reassigned
                        ctx.suggestions_locked = true;
                        ctx.status_message = ("Applied suggestion: swapped member from Team " + std::to_string(s.teamA + 1) + " with Team " + std::to_string(s.teamB + 1) + ".");
                    }
                }

                // button for viewing teams
                else if (label == "View Teams")
                {
                    write_line("Viewing teams.");
                }

                // button for quitting the program
                else if (label == "Quit")
                {
                    ctx.running = false;
                }
            }
        }

        // draw the UI 
        clear_screen(COLOR_WHITE);

        // Draw buttons in the list
        for (int i = 0; i < ctx.buttons.size(); i++)
        {
            draw_button(ctx.buttons[i]);
        }

        // compute totals for teams

        // positions of where the side panel should be
        float stat_x = AREA_X_START + AREA_W + 20.0;
        float stat_y = 48.0;
        float stat_w = 200.0;
        float stat_h = 140.0;

        // vector to hold score for each team
        vector<int> totals;

        // count how many teams have missing leaders
        int missing_leaders = 0;
        for (int ti = 0; ti < ctx.teams.size(); ti++)
        {
            totals.push_back(ctx.teams[ti].total_score);
            if (!ctx.teams[ti].hasLeader)
            {
                missing_leaders++;
            }
        }

        // FROM HERE I WILL CALCULATE METRICS

        // calculate the average of the teams (adding part)
        double mean = 0.0;
        for (int z = 0; z < totals.size(); z++)
        {
            mean += totals[z];
        }

        // divide by number of teams
        if (!totals.empty())
        {
            mean /= totals.size();
        }

        // calculate variance and standard deviation
        double var = 0.0;
        for (int z = 0; z < totals.size(); z++)
        {
            double d = totals[z] - mean;
            var += d * d;
        }

        if (!totals.empty())
        {
            var /= totals.size();
        }

        // standard deviation calculation
        double stddev = sqrt(var);

        // coordinates for suggestions box

        float sugg_x = stat_x;
        float sugg_y = stat_y + stat_h + 20.0;
        float sugg_w = stat_w;

        // suggestions box should fill available vertical space under the stats panel
        float sugg_h = WIN_H - (sugg_y + 20.0);

        // there should be a minimum height
        if (sugg_h < 80.0)
        {
            sugg_h = 80.0;
        }

        // this will draw a light grey background with a black border for aesthetics for the suggestions panel
        fill_rectangle(rgb_color(245, 245, 245), sugg_x - 6.0, sugg_y - 18.0, sugg_w + 12.0, sugg_h + 24.0);
        draw_rectangle(COLOR_BLACK, sugg_x - 6.0, sugg_y - 18.0, sugg_w + 12.0, sugg_h + 24.0);
        draw_text("Suggestions:", COLOR_BLACK, sugg_x, sugg_y - 12.0);

        // each suggestion should take about 60px of y axis space
        const float suggestion_slot_h = 60.0;
        int maxShow = std::floor((sugg_h - 8.0) / suggestion_slot_h); // floor is used to identify how many fit
        if (maxShow < 1)
        {
            maxShow = 1;
        }

        // draw each suggestion in lines
        for (int i = 0; i < ctx.suggestions.size() && i < maxShow; i++)
        {
            const SwapSuggestion &s = ctx.suggestions[i];

            // error handling (SO IMPORTANT!!!)
            std::string nameA = "(unknown)";
            std::string nameB = "(unknown)";
            int teamA = s.teamA;
            int teamB = s.teamB;
            int idxA = s.idxA;
            int idxB = s.idxB;

            // get names from the team data
            if (teamA >= 0 && teamA < ctx.teams.size() && idxA >= 0 && idxA < ctx.teams[teamA].members.size())
            {
                nameA = ctx.teams[teamA].members[idxA].name;
            }

            if (teamB >= 0 && teamB < ctx.teams.size() && idxB >= 0 && idxB < ctx.teams[teamB].members.size())
            {
                nameB = ctx.teams[teamB].members[idxB].name;
            }

            // here is where suggestion should appear
            float y_offset = sugg_y + i * suggestion_slot_h;

            // display suggestion to the user
            draw_text("Suggestion " + std::to_string(i + 1) + ":", COLOR_BLACK, sugg_x + 4, y_offset);
            draw_text(" Swap " + nameA + " (Team " + std::to_string(teamA + 1) + ")", COLOR_BLACK, sugg_x + 8, y_offset + 16);
            draw_text(" with " + nameB + " (Team " + std::to_string(teamB + 1) + ")", COLOR_BLACK, sugg_x + 8, y_offset + 32);

            // show improvement only if it's not 0
            double imp = s.delta;

            // i trialed and errored for this one and landed on this number
            if (imp > 0.000000001 || imp < -0.000000001)
            {
                std::string text = "Improvement: " + std::to_string(imp);
                draw_text(text, rgb_color(0, 100, 0), sugg_x + 8, y_offset + 48);
            }
        }

        // Draw teams grid for a guideline on where the team cards are to be placed
        float area_x = AREA_X_START;
        float area_y = AREA_Y_TOP;
        float area_w = AREA_W;
        float area_h = AREA_H;

        // adjust y position based on how far user has scrolled
        float scrolled_y_start = area_y - ctx.scroll_offset_y;

        // draw the stats panel

        // draw background box
        fill_rectangle(rgb_color(250, 250, 250), stat_x, stat_y, stat_w, stat_h);
        draw_rectangle(COLOR_BLACK, stat_x, stat_y, stat_w, stat_h);

        // draw texts
        draw_text("Team Statistics:", COLOR_BLACK, stat_x + 8.0, stat_y + 8.0);

        draw_text("Teams: " + std::to_string(ctx.teams.size()), COLOR_BLACK, stat_x + 8.0, stat_y + 30.0);

        draw_text("Missing leaders: " + std::to_string(missing_leaders), COLOR_BLACK, stat_x + 8.0, stat_y + 48.0);

        draw_text("Variance: " + std::to_string(var), COLOR_BLACK, stat_x + 8.0, stat_y + 70.0);

        draw_text("Std dev: " + std::to_string(stddev), COLOR_BLACK, stat_x + 8.0, stat_y + 92.0);

        // pass fixed areas into this function to draw the team cards
        draw_teams_grid(ctx.teams, area_x, scrolled_y_start, area_w, area_h, area_y);

        // updating status messages

        std::string status = ctx.status_message;

        // if no status message, show this message to handle for this case
        if (status.empty())
        {
            status = "Students: " + std::to_string(ctx.students.size()) + " | Teams: " + std::to_string(ctx.teams.size());
        }

        // call wrap function to convert the long message into multiple lines
        std::vector<std::string> lines = wrap_text(status, 27);
        const float STATUS_START_Y = 456.0;
        float y = STATUS_START_Y;

        // draw each wrapped line on screen
        for (int i = 0; i < lines.size(); i++)
        {
            draw_text(lines[i], COLOR_BLACK, 12.0, y);
            y += 18.0;
        }

        // draw live typed text box
        if (reading_text())
        {
            // draw white textbox
            fill_rectangle(color_white(), ctx.input_rect.x, ctx.input_rect.y, ctx.input_rect.width, ctx.input_rect.height);

            // draw the border around the textbox
            draw_rectangle(COLOR_BLACK, ctx.input_rect.x, ctx.input_rect.y, ctx.input_rect.width, ctx.input_rect.height);

            // show the user what they've typed so far
            draw_collected_text(COLOR_BLACK, font_named("input"), 18, option_defaults());
        }


        // refresh screen
        refresh_screen(60);

        // reset the pressed states of button so the next frame is clean!
        for (int i = 0; i < ctx.buttons.size(); i++)
        {
            ctx.buttons[i].pressed = false;
        }
    }

    // close all windows once loop ends (user quits)
    close_all_windows();
}
