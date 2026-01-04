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