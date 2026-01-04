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