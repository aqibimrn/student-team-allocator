// including relavent libraries
#pragma once
#include "structs.h"
#include <vector>
#include <string>
#include "splashkit.h"
#include <sstream>
#include "optimizer.h"

// a struct for button data
struct UIButton
{
    float x, y, w, h;
    std::string label;
    bool pressed;
};

/**
 * a struct that handles all things shown on the screen
 */
struct UIContext
{
    std::vector<student> students;
    std::vector<team> teams;
    std::vector<SwapSuggestion> suggestions;
    int chosenSuggestionIndex;
    std::vector<UIButton> buttons;
    bool running;
    std::string status_message;
    bool reading_csv;
    bool reading_teams;
    rectangle input_rect;
    std::string current_input;
    float scroll_offset_y;
    float max_scroll_y;
    bool suggestions_locked;
};

// initalize UI
void ui_init(UIContext &ctx);

// run the ui
void ui_run(UIContext &ctx);

// clear everything in the UI
void ui_cleanup(UIContext &ctx);