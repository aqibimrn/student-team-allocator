#pragma once
#include "structs.h"
#include <vector>

// Generate up to "max_suggestions" suggestions (best improvements).
void generate_swap_suggestions(const std::vector<team> &teams, int max_suggestions, std::vector<SwapSuggestion> &out_suggestions);
