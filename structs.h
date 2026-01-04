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