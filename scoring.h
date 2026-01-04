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
