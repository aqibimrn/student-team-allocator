// including relevant libraries
#include "structs.h"
#include "splashkit.h"
#include "allocator.h"
#include <vector>

using std::to_string;
using std::vector;

/**
 * choose the team with the lowest score (best index in this case)
 */
int choose_best_team_index(const vector<team> &teams)
{
    // defensive: if no teams, return -1
    if (teams.empty())
    {
        return -1;
    }

    int best_index = 0;
    int best_score = teams[0].total_score;
    int best_size = teams[0].size;

    // loop and check for the team with the lowest score
    for (int i = 1; i < teams.size(); i++)
    {
        int sc = teams[i].total_score;
        int sz = teams[i].size;

        // choose team with the lowest score, or smaller size if the scores end up tied
        if (sc < best_score || (sc == best_score && sz < best_size))
        {
            best_index = i;
            best_score = sc;
            best_size = sz;
        }
    }

    // return the index of the team with the lowest score
    return best_index;
}

/**
 * allocate the teams
 */
vector<team> allocate_teams(vector<student> students, int num_teams)
{
    vector<team> teams;

    // must have at least one team
    if (num_teams <= 0)
    {
        write_line("Number of teams must be > 0.");
        return teams;
    }

    // intialize each team
    teams.resize(num_teams);
    for (int i = 0; i < num_teams; i++)
    {
        teams[i].members.clear();
        teams[i].size = 0;
        teams[i].total_score = 0;
        teams[i].hasLeader = false;
        teams[i].id = i + 1;
    }

    // number of students available
    int n = students.size();

    // error handling for if no students exist
    if (n == 0)
    {
        write_line("No students provided.");
        return teams;
    }

    // main loop
    while (n > 0)
    {
        // find highest scoring student
        int best_student_index = 0;
        int best_student_score = students[0].student_score;
        for (int i = 1; i < n; i++)
        {
            if (students[i].student_score > best_student_score)
            {
                best_student_index = i;
                best_student_score = students[i].student_score;
            }
        }

        // pick best team
        int team_index = choose_best_team_index(teams);
        if (team_index == -1)
        {
            team_index = 0;
        }

        // assign student to chosen team
        teams[team_index].members.push_back(students[best_student_index]);
        teams[team_index].size = teams[team_index].members.size();
        teams[team_index].total_score += students[best_student_index].student_score;

        // mark if this team has a leader now
        if (students[best_student_index].leadership >= LEADER_THRESHOLD)
        {
            teams[team_index].hasLeader = true;
        }

        // remove student from eligible students to swap from
        if (best_student_index != n - 1)
        {
            student temp = students[best_student_index];
            students[best_student_index] = students[n - 1];
            students[n - 1] = temp;
        }
        students.pop_back();
        n = n - 1;
    }

    // final message confirming how many teams were formed
    write_line("Team allocation finished: " + to_string(num_teams) + " teams formed.");
    return teams;
}

/**
 * ensure every team has a leader
 */
void ensure_leader_present(vector<team> &teams)
{
    int k = teams.size();

    if (k == 0)
    {
        return;
    }

    // compute which teams have at least one leader
    for (int i = 0; i < k; i++)
    {
        bool found = false;

        // go thru all members of this team to find which teams have at least one leader
        for (int j = 0; j < teams[i].members.size(); j++)
        {
            if (teams[i].members[j].leadership >= LEADER_THRESHOLD)
            {
                found = true;
                break;
            }
        }

        teams[i].hasLeader = found;
    }

    // put teams with a missing leader into another vector
    vector<int> missing;
    for (int i = 0; i < k; i++)
    {
        if (!teams[i].hasLeader)
        {
            missing.push_back(i);
        }
    }

    if (missing.empty())
    {
        write_line("All teams already have a leader.");
        return;
    }

    // find teams with more than one eligble leader to act as a donor team

    vector<int> donors;

    for (int i = 0; i < k; i++)
    {
        int leaderCount = 0;
        for (int j = 0; j < teams[i].members.size(); j++)
        {
            if (teams[i].members[j].leadership >= LEADER_THRESHOLD)
            {
                leaderCount++;
            }
        }
        if (leaderCount > 1)
        {
            donors.push_back(i);
        }
    }

    // swap so that every team has a leader
    for (int mi = 0; mi < missing.size(); mi++)
    {
        int missindex = missing[mi];
        bool fixed = false;

        for (int di = 0; di < donors.size(); di++)
        {
            int donorindex = donors[di];

            if (donorindex == missindex)
            {
                continue;
            }

            // find a leader in donor
            int leaderPos = -1;
            for (int p = 0; p < teams[donorindex].members.size(); p++)
            {
                if (teams[donorindex].members[p].leadership >= LEADER_THRESHOLD)
                {
                    leaderPos = p;
                    break;
                }
            }

            // error handling (and a just in case factor!)

            if (leaderPos == -1)
            {
                continue;
            }

            // find a non-leader in missing team
            int nonLeaderPos = -1;

            for (int p = 0; p < teams[missindex].members.size(); p++)
            {
                if (teams[missindex].members[p].leadership < LEADER_THRESHOLD)
                {
                    nonLeaderPos = p;
                    break;
                }
            }

            // crash handling, although this shouldn't happen
            if (nonLeaderPos == -1)
            {
                continue;
            }

            // perform the swap
            student temp = teams[donorindex].members[leaderPos];
            teams[donorindex].members[leaderPos] = teams[missindex].members[nonLeaderPos];
            teams[missindex].members[nonLeaderPos] = temp;

            // recompute donor team stats
            teams[donorindex].total_score = 0;
            teams[donorindex].hasLeader = false;
            for (int p = 0; p < teams[donorindex].members.size(); p++)
            {
                teams[donorindex].total_score += teams[donorindex].members[p].student_score;
                if (teams[donorindex].members[p].leadership >= LEADER_THRESHOLD)
                    teams[donorindex].hasLeader = true;
            }

            teams[donorindex].size = teams[donorindex].members.size();

            // recompute missing team stats
            teams[missindex].total_score = 0;
            teams[missindex].hasLeader = false;

            for (int p = 0; p < teams[missindex].members.size(); p++)
            {
                teams[missindex].total_score += teams[missindex].members[p].student_score;
                if (teams[missindex].members[p].leadership >= LEADER_THRESHOLD)
                    teams[missindex].hasLeader = true;
            }

            teams[missindex].size = teams[missindex].members.size();

            write_line("Swapped a leader from Team " + to_string(donorindex + 1) + " to Team " + to_string(missindex + 1));
            fixed = true;
            break;
        }

        if (!fixed)
        {
            write_line("Could not find a leader swap for Team " + to_string(missindex + 1) + ". Not enough donors.");
        }
    }
}

/**
 * recalculates stats for team (such as total_score, size and hasLeader)
 */
void recompute_team_stats(team &t)
{
    // reset total score
    t.total_score = 0;
    // update size to number of members
    t.size = t.members.size();
    // reset leader boolean
    t.hasLeader = false;

    // loop through all memebers to calculate totals
    for (int i = 0; i < t.members.size(); i++)
    {
        // sum all members score
        t.total_score += t.members[i].student_score;

        // if any member qualifies as a leader, then this will update the leader status of team to true
        if (t.members[i].leadership >= LEADER_THRESHOLD)
        {
            t.hasLeader = true;
        }
    }
}

// including relavent libraries
#pragma once
#include "structs.h"
#include <vector>
#include <sstream>


using std::vector;

/**
 * choose team with lowest score (aka best index)
 */
int choose_best_team_index(const vector<team> &teams);

/**
 * Allocate students into num_teams using the algorithm
 */
vector<team> allocate_teams(vector<student> students, int num_teams);

/**
 * Ensure every team has at least one leader (leadership >= leader_threshold)
 */
void ensure_leader_present(vector<team> &teams);

void recompute_team_stats(team &t);

#include "io.h"
#include "splashkit.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>


using std::vector;
using std::ifstream;
using std::stringstream;
using std::string;

/**
 * trim the space between the two ends
 */
std::string trim_string(const std::string &s)
{
    int start = 0;
    int end = s.size() - 1;

    // move start forward while current char is whitespace
    while (start <= end && std::isspace(s[start]))
    {
        start++;
    }

    // similarly move end backward while current char is whitespace
    while (end >= start && std::isspace(s[end]))
    {
        end--;
    }

    // if the string is all whitespace return the original, empty string
    if (end < start) 
    {
        return std::string();
    }

    // return a substring from start to the end
    return s.substr(start, end - start + 1);
}

/**
 * convert string to integer using catch if it fails to avoid crashing
 */
int safe_stoi(const std::string &token, int fallback)
{
    std::string t = trim_string(token); // trim first

    // error handling
    if (t.empty())
    {
        return fallback;
    }

    // using catch as taught in programmers.guide
    try
    {
        size_t idx = 0;
        long value = std::stol(t, &idx);
        if (idx != t.size()) 
        {
            return fallback;
        }

        return value;
    }

    catch (...)
    {
        return fallback; 
    }
}

/**
 * load students from CSV file
 */
std::vector<student> load_students_from_csv(const std::string &filename)
{
    std::vector<student> students;
    std::ifstream file(filename.c_str());

    // error handling
    if (!file.is_open())
    {
        write_line("Error: Could not open file: " + filename);
        return students;
    }

    // each line from file is held in 'line'
    std::string line;
    // which line we're on
    int line_no = 1;

    // Skip the first line which is the headers
    std::getline(file, line);

    // read file line by line
    while (std::getline(file, line))
    {
        // trim the whitespace around the line
        std::string raw = trim_string(line);

        // skip the line if its blank
        if (raw.empty())
        {
            line_no++;
            continue;
        }

        // create string stream to separate values between commas
        std::stringstream ss(line);
        student s;
        std::string token;

        // separate names
        // skip the line if its missing data (error handling)
        if (!std::getline(ss, token, ','))
        {
            write_line("Skipping skewed CSV line " + std::to_string(line_no));
            line_no++;
            continue;
        }
        // store the student's name (trimmed)
        s.name = trim_string(token);

        // store student's leadership score

        if (!std::getline(ss, token, ',')) 
        {
            // error handling, if leadership score is missing then default to 0
            s.leadership = 0; 
        }
        else 
        {
            // convert safely to integer
            s.leadership = stoi(token, 0);
        }

        // frontend score
        if (!std::getline(ss, token, ',')) 
        {
            // error handling
            s.frontend = 0;
        }
        else 
        {
            // convert integer safely
            s.frontend = stoi(token, 0);
        }

        // backend scroe
        if (!std::getline(ss, token, ',')) 
        {
            s.backend = 0;
        }
        else 
        {
            s.backend = stoi(token, 0);
        }

        // security score
        if (!std::getline(ss, token, ',')) 
        {
            s.security   = 0;
        }
        else
        {
            s.security   = stoi(token, 0);
        } 

        // ui/ux score
        if (!std::getline(ss, token, ','))
        {
            s.ui = 0;
        }  
        else 
        {
            s.ui = stoi(token, 0);
        }

        // english score
        if (!std::getline(ss, token, ',')) 
        {
            s.english = 0;
        } 
        else 
        {
            s.english = stoi(token, 0);
        }
        
        // intializing values
        s.student_score = 0;
        s.x = 0.0;
        s.y = 0.0;
        s.selected = false;

        students.push_back(s);
        line_no++;
    }

    // success message & close the file

    file.close();
    write_line("Loaded " + std::to_string(students.size()) + " students from " + filename);
    return students;
}

// importing libraries
#pragma once
#include "structs.h"
#include <vector>
#include <string>

using std::vector;
using std::string;


std::string trim_string(const std::string &s);

int safe_stoi(const std::string &token, int fallback);

vector<student> load_students_from_csv(const string &filename);


// importing the libraries
#include "structs.h"
#include "splashkit.h"
#include "ui.h"
#include <string>
#include <sstream>

// main function
int main()
{
    // create UIContext object
    UIContext ctx;

    // intialize the UI interface
    ui_init(ctx);
    ui_run(ctx);
    ui_cleanup(ctx);

// return 0 to indicate a successful program execution!
    return 0;
}


#include "optimizer.h"
#include <cmath>
#include <string>

using std::vector;
using std::to_string;

// compute mean of integers in vector
double mean_int_vector(const vector<int> &vals)
{
    if (vals.empty()) 
    {
        return 0.0;
    }

    double sum = 0.0;

    for (int vi = 0; vi < vals.size(); vi++)
    {
        sum += vals[vi];
    } 

    return (sum / vals.size());
}

// compute variance of integers in vector
double variance_int_vector(const vector<int> &vals)
{
    if (vals.empty()) 
    {
        return 0.0;
    }

    double mean_value = mean_int_vector(vals);

    double accum = 0.0; // accumulator for squared differences

    // loop thru all elements
    for (int vi = 0; vi < vals.size(); vi++)
    {
        double diff = vals[vi] - mean_value; // difference from mean
        accum += diff * diff; // add squared difference 
    }

    // return average square difference
    return accum / vals.size();
}

// compute balance metric described in plan
double compute_balance_metric(const vector<team> &teams)
{
    // vector to store total scores fo all teams
    vector<int> totals;

    // loop thru all teams and collect total scores
    for (int teamIdx = 0; teamIdx < teams.size(); teamIdx++)
    {
        totals.push_back(teams[teamIdx].total_score);
    }

    // compute variance of team totals
    double var = variance_int_vector(totals);

    // count teams missing leaders
    int missing = 0;

    // check every team to see if they have a leader or not
    for (int teamIdx = 0; teamIdx < teams.size(); teamIdx++)
    {
        if (!teams[teamIdx].hasLeader) 
        {
            missing++;
        }
    }

    // a penalty for missing leaders
    const double LEADER_PENALTY = 1000.0; 

    // combine variance and penalty
    double metric = var + LEADER_PENALTY * missing;

    // return metric
    return metric;
}

// Insert swap suggestions into a store vector
void insert_suggestion_sorted(vector<SwapSuggestion> &out_suggestions, const SwapSuggestion &sugg, int max_suggestions)
{
    // intitalize insertion index
    int pos = 0;

    // find correct point to insert and move forward while current delta is smaller
    while (pos < out_suggestions.size() && out_suggestions[pos].delta <= sugg.delta)
    {
        pos++;
    }

    // insert at pos if we've reached the end
    if (pos >= out_suggestions.size())
    {
        out_suggestions.push_back(sugg);
    }

    else
    {
        // insert in middle, duplicate last element to extend the vector
        out_suggestions.push_back(out_suggestions[out_suggestions.size() - 1]);

        // shift elements to the right to make room
        for (int shiftIdx = out_suggestions.size() - 2; shiftIdx > pos; shiftIdx--)
        {
            out_suggestions[shiftIdx] = out_suggestions[shiftIdx - 1];
        }

        // place new suggestion in the correct position
        out_suggestions[pos] = sugg;
    }

    // Make sure vector doesn't exceed max_suggestions
    while (out_suggestions.size() > max_suggestions)
    {
        out_suggestions.pop_back(); // remove the last (worst) suggestion
    }

}

// generate suggestions to swap and improve balance between teams
void generate_swap_suggestions(const vector<team> &teams, int max_suggestions, vector<SwapSuggestion> &out_suggestions)
{
    // empty suggestions list
    out_suggestions.clear();

    // total number of teams
    int teamCount = teams.size();

    // error handling
    if (teamCount <= 1) 
    {
        return;
    }
    
    // compute the current balance metric
    double base_metric = compute_balance_metric(teams);

    // loop through all unique pairs of teams (A and B)
    for (int teamAIndex = 0; teamAIndex < teamCount; teamAIndex++)
    {
        for (int teamBIndex = teamAIndex + 1; teamBIndex < teamCount; teamBIndex++)
        {
            int sizeA = teams[teamAIndex].members.size();
            int sizeB = teams[teamBIndex].members.size();

            // skip empty teams
            if (sizeA == 0 || sizeB == 0) 
            {
                continue;
            }

            // try swapping every pair for members between A and B
            for (int memberAIndex = 0; memberAIndex < sizeA; memberAIndex++)
            {
                for (int memberBIndex = 0; memberBIndex < sizeB; memberBIndex++)
                {
                    // compute new totals and leader presence
                    int oldA_total = teams[teamAIndex].total_score;
                    int oldB_total = teams[teamBIndex].total_score;

                    int scoreA = teams[teamAIndex].members[memberAIndex].student_score;
                    int scoreB = teams[teamBIndex].members[memberBIndex].student_score;

                    int newA_total = oldA_total - scoreA + scoreB;
                    int newB_total = oldB_total - scoreB + scoreA;

                    // copy teams totals into a temporary vector of ints to compute metric
                    vector<int> tempTotals;

                    for (int teamIdx = 0; teamIdx < teams.size(); teamIdx++)
                    {
                        if (teamIdx == teamAIndex) 
                        {
                            // replace team A's total
                            tempTotals.push_back(newA_total);
                        }

                        // replace team B's total
                        else if (teamIdx == teamBIndex) 
                        {
                            tempTotals.push_back(newB_total);
                        }

                        // keep other teams the same
                        else 
                        {
                            tempTotals.push_back(teams[teamIdx].total_score);
                        }
                    }

                    // compute leader missing count after swap
                    int missing_after = 0;

                    for (int teamIdx = 0; teamIdx < teams.size(); teamIdx++)
                    {
                        bool hasLeader = teams[teamIdx].hasLeader;

                        if (teamIdx == teamAIndex)
                        {
                            // after swap, check if team A has leader
                            bool leaderFound = false;

                            for (int memberIdx = 0; memberIdx < teams[teamAIndex].members.size(); memberIdx++)
                            {
                                if (memberIdx == memberAIndex)
                                {
                                    // check if swapped-in member from team B is a leader or not
                                    if (teams[teamBIndex].members[memberBIndex].leadership >= LEADER_THRESHOLD) 
                                    {
                                        leaderFound = true;
                                    }
                                }

                                // unchanged members remain in team A
                                else
                                {
                                    if (teams[teamAIndex].members[memberIdx].leadership >= LEADER_THRESHOLD) 
                                    {
                                        leaderFound = true;
                                    }
                                }
                            }
                            // update team A's leader status
                            hasLeader = leaderFound;
                        }

                        // evaluate team B after the swap
                        else if (teamIdx == teamBIndex)
                        {
                            bool leaderFound = false;
                            for (int memberIdx = 0; memberIdx < teams[teamBIndex].members.size(); memberIdx++)
                            {
                                if (memberIdx == memberBIndex)
                                {
                                    // check if incoming memebr from A is a leader
                                    if (teams[teamAIndex].members[memberAIndex].leadership >= LEADER_THRESHOLD) 
                                    {
                                        leaderFound = true;
                                    }
                                }
                                else
                                {
                                    if (teams[teamBIndex].members[memberIdx].leadership >= LEADER_THRESHOLD) 
                                    {
                                        leaderFound = true;
                                    }
                                }
                            }
                            // update team B's leadership status
                            hasLeader = leaderFound;
                        }

                        // count missing leaders
                        if (!hasLeader) 
                        {
                            missing_after++;
                        }
                    }

                    // compute variance for tempTotals
                    double mean_value = 0.0;

                    for (int tIdx = 0; tIdx < tempTotals.size(); tIdx++) mean_value += tempTotals[tIdx];
                    mean_value = mean_value / tempTotals.size();

                    double accum = 0.0;

                    for (int tIdx = 0; tIdx < tempTotals.size(); tIdx++)
                    {
                        double diff = tempTotals[tIdx] - mean_value;
                        accum += diff * diff;
                    }

                    // new variance is stored in var_after
                    double var_after = accum / tempTotals.size();

                    // same penalty contant
                    const double LEADER_PENALTY = 1000.0;
                    double metric_after = var_after + LEADER_PENALTY * missing_after;

                    double delta = metric_after - base_metric;

                    // create suggestion
                    SwapSuggestion s;
                    s.teamA = teamAIndex;
                    s.idxA = memberAIndex;
                    s.teamB = teamBIndex;
                    s.idxB = memberBIndex;
                    s.delta = delta;

                    // only consider if delta is improvement (negative) OR top few even if positive
                    insert_suggestion_sorted(out_suggestions, s, max_suggestions);
                }
            }
        }
    }
}

#pragma once
#include "structs.h"
#include <vector>

// Generate up to "max_suggestions" suggestions (best improvements).
void generate_swap_suggestions(const std::vector<team> &teams, int max_suggestions, std::vector<SwapSuggestion> &out_suggestions);

// importing libraries
#include "structs.h"
#include "splashkit.h"
#include "scoring.h"
#include "utilities.h"
#include <vector>

using std::to_string;
using std::vector;

/**
 * A function to calculate the score of a single student
 */
int compute_student_score_int(const student &s)
{
    double score = 0.0;
    score += s.english * W_ENG;
    score += s.frontend * W_FRONT;
    score += s.backend * W_BACK;
    score += s.security * W_SEC;
    score += s.ui * W_UI;
    score += s.leadership * W_LEAD;

    return int(score);
}


/**
 * computing scores for every student
 */
void compute_scores_for_all(vector<student> &students)
{
    // compute and store student_score for each student
    for (int i = 0; i < students.size(); i++)
    {
        students[i].student_score = compute_student_score_int(students[i]);
    }

    write_line("Computed scores for " + to_string(students.size()) + " students.");
    write_line();

    // print all students with their scores
    write_line("Students and computed scores :");
    for (int i = 0; i < students.size(); i++)
    {
        write_line(to_string(i + 1) + ". " + students[i].name + " | score: " + to_string(students[i].student_score) + " | leadership: " + to_string(students[i].leadership));
    }
}

/**
 * checks if the student is eligible to be a leader or not
 */
bool is_leader(const student &s, int leader_threshold)
{
    return s.leadership >= leader_threshold;
}

// including relevant libraries
#pragma once
#include "structs.h"
#include <vector>
#include <sstream>

using std::vector;

// constant weightages
const double W_ENG   = 0.15;
const double W_FRONT = 0.20;
const double W_BACK  = 0.20;
const double W_SEC   = 0.15;
const double W_UI    = 0.15;
const double W_LEAD  = 0.15;


// Compute a single student's integer score from their skill fields.
int compute_student_score_int(const student &s);

// Compute scores for every student in the vector and store into .student_score.
void compute_scores_for_all(vector<student> &students);

// True if student's leadership value meets or exceeds the threshold.
bool is_leader(const student &s, int leader_threshold);

// importing main libraries
#pragma once
#include <string>
#include <vector>


// constant leader threshold (student must have a score of greater than or equal to 7 to be eligible to be a leader)
const int LEADER_THRESHOLD = 7;

// student struct
struct student {
    std::string name;
    int leadership;
    int frontend;
    int backend;
    int security;
    int ui;
    int english;
    int student_score;
    // locations of where to place
    float x;
    float y;
    bool selected;
};

// team struct
struct team {
    std::vector<student> members;
    int size;            
    int total_score;     
    bool hasLeader;      
    int id;              
    // locations of where to place
    float x;
    float y;
    float width;
    float height;
};

// swap suggestion struct
struct SwapSuggestion {
    int teamA;
    int idxA;
    int teamB;
    int idxB;
    double delta; // newchanges - old changes (negative = improvement) a way to keep track if its beneficial to swap or not
};

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

#ifndef UTILITIES_H
#define UTILITIES_H
#include "structs.h"
#include "visualizer.h"

#include <string>
using std::string;

/**
 * Read a string from the user
 *
 * @param prompt the message to show the user
 * @returns the string entered
 */
string read_string(string prompt);

/**
 * Read an integer from the user
 *
 * @param prompt the message to show the user
 * @returns the integer entered
 */
int read_integer(string prompt);

/**
 * Read a double from the user
 *
 * @param prompt the message to show the user
 * @returns the double entered
 */
double read_double(string prompt);

/**
 * Calculate all the team heights
 */
float calculate_total_teams_display_height(const std::vector<team> &teams);


#endif

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
