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
