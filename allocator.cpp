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