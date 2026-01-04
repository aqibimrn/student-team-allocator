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
