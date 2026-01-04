#include "utilities.h"
#include "splashkit.h"
#include "structs.h"

string read_string(string prompt)
{
  write(prompt);
  return read_line();
}

int read_integer(string prompt)
{
  string line = read_string(prompt);
  while (!is_integer(line))
  {
    write_line("Please enter a whole number.");
    line = read_string(prompt);
  }
  return stoi(line);
}

double read_double(string prompt)
{
  string line = read_string(prompt);
  while (!is_double(line))
  {
    write_line("Please enter a double number.");
    line = read_string(prompt);
  }
  return stod(line);
}

/**
 * a function to calculate the total height of display for all teams. it loops thru each team and figures its height, then sums them all up!
 */
float calculate_total_teams_display_height(const std::vector<team> &teams)
{
  // error handling
    if (teams.empty())
    {
        return 0.0;
    }

    // constants declarations
    const int per_row = 2;  // we can change the number of cards per row from here      
    const float chip_h = 26.0; // height of each student inside a card
    const float header_h = 48.0; // height of team header
    const float bottom_padding = 16.0; // space at the bottom of each card so it looks neat
    const float spacing = 12.0; // spacing between the rows
    const float max_card_h = 400.0; // limit of each card height

    // number of teams per row 
    int num_teams = teams.size();
    int num_rows = (num_teams + per_row - 1) / per_row;

    // compute each card height and row max
    std::vector<float> row_max_height(num_rows, 0.0);

    // for loop to figure out how tall each card needs to be
    for (int i = 0; i < num_teams; i++)
    {
        int members = teams[i].members.size();

        // calculation for total card height (consists of headers + (size of number of students) + padding)
        float h = header_h + (members * (chip_h + 6.0)) + bottom_padding;

        // max height
        if (h > max_card_h) 
        {
          h = max_card_h;
        }

        int row = i / per_row;

        // using array to store the height
        if (h > row_max_height[row]) 
        {
          row_max_height[row] = h;
        }
    }

    // sum all heights
    float total = spacing;
    for (int r = 0; r < num_rows; r++)
    {
        total += row_max_height[r];
        total += spacing;
    }

    return total;
}
